/*
zip
read from a zip file

Copyright (C) 2009 Evan Rinehart

This software comes with no warranty.
1. This software can be used for any purpose, good or evil.
2. Modifications must retain this license, at least in spirit.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <zlib.h>

#include <zip.h>

#define BUF_SIZE 1024
#define TABLE_SIZE 128
#define EBUF_SIZE 256

struct record {
	char* filename;
	unsigned offset;
	unsigned clen;
	unsigned ulen;
	unsigned method;
	struct record* next;
	struct record* contents;
};

struct zip_archive {
	int (*read)(void* userdata, byte buf[], int count);
	int (*seek)(void* userdata, int offset, int whence);
	void (*close)(void* userdata);
	void* userdata;

	struct record* table[TABLE_SIZE];
	int ptr;
};

struct zip_file {
	zip_archive* arc;
	int cptr, clen;
	int uptr, ulen;
	int offset;
	byte inbuf[BUF_SIZE];
	int eof;
	z_stream strm;
};

struct zip_dir {
	struct record* ptr;
};


static char errbuf[EBUF_SIZE] = "";


/* internal routines */

static void set_error(char* msg){
	strncpy(errbuf, msg, EBUF_SIZE);
	errbuf[EBUF_SIZE-1] = '\0';
}

static void out_of_memory(){
	set_error("OUT OF MEMORY");
}

/* http://www.cse.yorku.ca/~oz/hash.html */
static unsigned long hash(char* str){
	unsigned char* ptr = (unsigned char*)str;
	unsigned long hash = 5381;
	int c;
	while((c = *ptr++))
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	return hash;
}


static struct record* make_record(char* filename, int ulen, int clen, int offset, int method){
	struct record* r = malloc(sizeof(struct record));
	if(r == NULL){
		return NULL;
	}
	r->filename = strdup(filename);
	r->clen = clen;
	r->ulen = ulen;
	r->offset = offset;
	r->method = method;
	r->next = NULL;
	r->contents = NULL;
	return r;
}

static struct record* copy_record(struct record* rec){
	return make_record(rec->filename, rec->ulen, rec->clen, rec->offset, rec->method);
}

static struct record* get_record(zip_archive* arc, char* filename){
	int i = hash(filename) % TABLE_SIZE;
	struct record* ptr = arc->table[i];
	while(ptr){
		if(strcmp(filename, ptr->filename) == 0){
			return ptr;
		}
		ptr = ptr->next;
	}
	return NULL;
}

static void hash_insert(zip_archive* arc, struct record* rec){
	int i = hash(rec->filename) % TABLE_SIZE;
	struct record* ptr = arc->table[i];

	if(get_record(arc, rec->filename)){
		fprintf(stderr, "zip: can't insert %s twice\n", rec->filename);
		return;
	}

	if(ptr == NULL){
		arc->table[i] = rec;
		return;
	}

	rec->next = ptr;
	arc->table[i] = rec;
}

static char* baseof(char* path){
	if(strlen(path) < 2) return NULL;
	int ptr = strlen(path) - 2;
	while(ptr > 0 && path[ptr] != '/') ptr--;
	if(ptr == 0) return NULL;
	char* result = malloc(ptr+5);
	memcpy(result, path, ptr+1);
	result[ptr+1] = '\0';
	return result;
}

static void contents_insert(struct record* item, struct record* dir){
	item->next = dir->contents;
	dir->contents = item;
}
/*
static void print_hash(zip_archive* arc){
	int i;
	struct record* rec;
	for(i=0; i<TABLE_SIZE; i++){
		printf("table[%d]: ",i);
		rec = arc->table[i];
		while(rec){
			printf("%p ", rec);
			rec = rec->next;
		}
		printf("\n");
	}
}

static void print_zipfile(zip_file* f){
	printf("(+0x%x, %u/%uB, cptr +0x%x, uptr %u)\n",
	f->offset, f->clen, f->ulen, f->cptr, f->uptr);
}

static void print_record(struct record* rec){
	printf("(%s, %uB, %uB, +0x%x, method %u)\n", rec->filename, rec->clen, rec->ulen, rec->offset, rec->method);
}
*/
static char* method_str(unsigned method){
	switch(method){
		case 0: return "uncompressed";
		case 1: return "shrink";
		case 2:
		case 3:
		case 4:
		case 5: return "reduce";
		case 6: return "implode";
		case 8: return "deflate";
		case 9: return "deflate64";
		case 10: return "IBM TERSE (old)";
		case 12: return "bzip2";
		case 14: return "lzma";
		case 18: return "IBM TERSE (new)";
		case 19: return "IBM LZ77 z";
		case 97: return "WavPack";
		case 98: return "PPMd";
		default: return "unknown";
	}
}

static void unrecognized_method(unsigned method){
	char buf[64];
	snprintf(buf, 64, "unrecognized compression method '%s'", method_str(method));
	buf[63] = '\0';
	set_error(buf);
}

static void set_record(zip_archive* arc, char* filename, int ulen, int clen, int offset, int method){
	struct record* rec;
	struct record* dir;
	char* dirname;
	if(get_record(arc, filename)) return;
	rec = make_record(filename, ulen, clen, offset, method);
	hash_insert(arc, rec);
	dirname = baseof(filename);
	if(dirname == NULL) return;
	if(get_record(arc, dirname) == NULL)
		set_record(arc, dirname, 0, 0, 0, 0);
	dir = get_record(arc, dirname);
	contents_insert(copy_record(rec), dir);
	free(dirname);
}



static int read_bytes(zip_archive* arc, void* buf, int count){
	return arc->read(arc->userdata, buf, count);
}

static int read_chunk(zip_archive* arc, void* buf, int count){
	int n = read_bytes(arc, buf, count);
	if(n < 0){
		set_error("read error");
		return -1;
	}
	if(n < count){
		set_error("not enough data");
		return -1;
	}
	return 0;
}

static int skip(zip_archive* arc, int count){
	char buf[256];
	int n;
	int total = 0;

	while(total != count){
		int diff = count - total;
		int chunk = diff < 256 ? diff : 256;
		n = read_bytes(arc, buf, chunk);
		if(n < 0){
			set_error("read error");
			return -1;
		}
		if(n < chunk){
			set_error("not enough data");
			return -1;
		}
		total += n;
	}

	return 0;
}

static int read_long(zip_archive* arc, unsigned* n){
	unsigned char b[4];
	int e = read_bytes(arc, b, 4);
	if(e < 0){
		set_error("read error");
		return -1;
	}
	if(e < 4){
		set_error("not enough data");
		return -1;
	}
	*n = b[0] | (b[1]<<8) | (b[2]<<16) | (b[3]<<24);
	return 0;
}

static int read_short(zip_archive* arc, unsigned* n){
	unsigned char b[2];
	int e = read_bytes(arc, b, 2);
	if(e < 0){
		set_error("read error");
		return -1;
	}
	if(e < 2){
		set_error("not enough data");
		return -1;
	}
	*n = b[0] | (b[1]<<8);
	return 0;
}


static int parse_local_header(zip_archive* arc){
	unsigned L1, L2;
	char* filename;
	unsigned method;
	unsigned clen, ulen;
	unsigned offset;
	unsigned flags;

	if(
		skip(arc, 2) ||
		read_short(arc, &flags) ||
		read_short(arc, &method) ||
		skip(arc, 8) ||
		read_long(arc, &clen) ||
		read_long(arc, &ulen) ||
		read_short(arc, &L1) ||
		read_short(arc, &L2)
	){
		return -1;
	}

	filename = malloc(L1+1);
	if(
		read_chunk(arc, filename, L1) ||
		skip(arc, L2) ||
		skip(arc, clen)
	){
		free(filename);
		return -1;
	}

	filename[L1] = '\0';
	offset = arc->ptr + 30 + L1 + L2;
	arc->ptr = offset + clen + (flags&(1<<3) ? 12 : 0);

	set_record(arc, filename, ulen, clen, offset, method);

	free(filename);

	return 0;
}

static int build_directory(zip_archive* arc){
	while(1){
		unsigned sig;
		if(read_long(arc, &sig) < 0) return -1;
		else if(sig == 0x04034b50) parse_local_header(arc);
		else break;
	}

	return 0;
}


static void free_directory_r(struct record* r){
	if(r->contents) free_directory_r(r->contents);
	if(r->next) free_directory_r(r->next);
	free(r->filename);
	free(r);
}

static void free_directory(zip_archive* arc){
	int i;
	for(i=0; i<TABLE_SIZE; i++){
		if(arc->table[i]) free_directory_r(arc->table[i]);
	}  
}


static int fill_inbuf(zip_file* f){
	int nread, nwant;
	int nleftstream, nleftbuffer;

	/* shift everything left */
	memmove(f->inbuf, f->strm.next_in, f->strm.avail_in);
	f->strm.next_in = f->inbuf;

	/* fill the buffer */
	if(f->arc->seek(f->arc->userdata, f->offset+f->cptr, SEEK_SET) < 0){
		set_error("archive seek error");
		return -1;
	}

	/* you want the minimum of
	a) bytes needed to fill the buffer and
	b) bytes left in the compressed stream */
	nleftstream = f->clen - f->cptr;
	nleftbuffer = BUF_SIZE - f->strm.avail_in;
	nwant = nleftstream < nleftbuffer ? nleftstream : nleftbuffer;
	nread = f->arc->read(f->arc->userdata, f->inbuf+f->strm.avail_in, nwant);
	if(nread < 0){
		set_error("archive read error");
		return -1;
	}

	f->cptr += nread;
	f->strm.avail_in += nread;
	return nread;
}

static int inflate_chunk(zip_file* f, byte buf[], int count){
	f->strm.next_out = buf;
	f->strm.avail_out = count;
	int e = inflate(&f->strm, Z_SYNC_FLUSH);
	switch(e){
	case Z_OK:
		return count - f->strm.avail_out;
	case Z_STREAM_END:
		f->eof = 1;
		return count - f->strm.avail_out;
	case Z_BUF_ERROR:
		return 0;
	case Z_NEED_DICT:
		set_error("inflate needs a preset dictionary at this point");
		return -1;
	case Z_DATA_ERROR:
		set_error("inflate data error (input corrupted or in wrong format)");
		return -1;
	case Z_STREAM_ERROR:
		set_error("inflate stream error (inconsistent stream structure)");
		return -1;
	case Z_MEM_ERROR:
		set_error("inflate out of memory");
		return -1;
	default:
		set_error("inflate error (unknown)");
		return -1;
	}
}


/* these are callbacks for the default archive reader, filesystem i/o */
static int file_read(void* f, byte buf[], int count){
	return fread(buf, 1, count, f);
}

static int file_seek(void* f, int offset, int whence){
	return fseek(f, offset, whence);
}

static void file_close(void* f){
	fclose(f);
}







/* public methods */

zip_archive* zip_aropenf(char* filename){
	FILE* f = fopen(filename, "r");
	if(f == NULL){
		set_error("i/o error");
		return NULL;
	}
	zip_reader rd = {file_read, file_seek, file_close, f};
	return zip_aropen(&rd);
}

zip_archive* zip_aropen(zip_reader* rd){
	int i;
	zip_archive* arc = malloc(sizeof(zip_archive));
	if(arc == NULL){
		out_of_memory();
		return NULL;
	}
	arc->read = rd->read;
	arc->seek = rd->seek;
	arc->close = rd->close;
	arc->userdata = rd->userdata;
	arc->ptr = 0;

	for(i=0; i<TABLE_SIZE; i++){
		arc->table[i] = NULL;
	}

	if(build_directory(arc) < 0){
		free_directory(arc);
		free(arc);
		return NULL;
	}

	return arc;
}

void zip_arclose(zip_archive* arc){
	arc->close(arc->userdata);
	free_directory(arc);
	free(arc);
}

zip_file* zip_fopen(zip_archive* arc, char* path){
	struct record* r = get_record(arc, path);
	if(r == NULL){
		set_error("file not found");
		return NULL;
	}

	if(r->filename[strlen(r->filename)-1] == '/'){
		set_error("cannot open directory");
		return NULL;
	}

	if(r->method != 8){
		unrecognized_method(r->method);
		return NULL;
	}

	zip_file* f = malloc(sizeof(zip_file));
	if(f == NULL){
		out_of_memory();
		return NULL;
	}

	f->eof = 0;
	f->arc = arc;
	f->strm.zalloc = NULL;
	f->strm.zfree = NULL;
	f->strm.opaque = NULL;
	f->strm.next_in = f->inbuf;
	f->strm.avail_in = 0;
	f->strm.next_out = NULL;
	f->strm.avail_out = 0;
	f->clen = r->clen;
	f->cptr = 0;
	f->ulen = r->ulen;
	f->uptr = 0;
	f->offset = r->offset;

	int e = inflateInit2(&f->strm, -15);
	if(e != Z_OK){
		switch(e){
			case Z_MEM_ERROR: set_error("memory"); break;
			case Z_STREAM_ERROR: set_error("stream"); break;
		}
		free(f);
		return NULL;
	}

	return f;
}

void zip_fclose(zip_file* f){
	inflateEnd(&f->strm);
	free(f);
}

int zip_fread(zip_file* f, byte buf[], int count){
	int total = 0;
	int n;
	int sentry = 0; /* annoying */

	while(count > 0 && !f->eof){
		n = inflate_chunk(f, buf+total, count);

		if(n < 0) return -1;
		if(n == 0){
			/* need more input */
			if(fill_inbuf(f) < 0) return -1;
			if(sentry == 1){
				set_error("unable to satisfy buffer requirements");
				return -1;
			}
			sentry = 1;
			continue;
		}

		sentry = 0;
		count -= n;
		total += n;

		f->uptr += n;
		if(f->uptr == f->ulen){
			f->eof = 1;
		}
	}

	return total;
}

int zip_feof(zip_file* f){
	return f->eof;
}




zip_dir* zip_opendir(zip_archive* arc, char* path){
	if(path[strlen(path)-1] != '/'){
		set_error("path does not specify directory");
		return NULL;
	}

	zip_dir* dir = malloc(sizeof(zip_dir));
	if(dir == NULL){
		out_of_memory();
		return NULL;
	}

	struct record* r = get_record(arc, path);
	if(r == NULL){
		set_error("file not found");
		free(dir);
		return NULL;
	}

	dir->ptr = r->contents;
	return dir;
}

char* zip_readdir(zip_dir* dir){
	if(dir->ptr == NULL){
		/* no more entries, not an error */
		return NULL;
	}
	else{
		char* filename = dir->ptr->filename;
		dir->ptr = dir->ptr->next;
		return filename;
	}
}

void zip_closedir(zip_dir* dir){
	free(dir);
}


char* zip_geterror(){
	return errbuf;
}
