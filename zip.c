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

#define BUF_SIZE 256
#define TABLE_SIZE 128


struct record {
  char* filename;
  unsigned method;
  unsigned offset;
  unsigned clen;
  unsigned ulen;
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
  int ptr;
  int len;
  byte inbuf[BUF_SIZE];
  int eof;
  z_stream strm;
};

struct zip_dir {
  struct record* ptr;
};





/* internal routines */


/* http://www.cse.yorku.ca/~oz/hash.html */
static unsigned long hash(char* str){
  unsigned char* ptr = (unsigned char*)str;
  unsigned long hash = 5381;
  int c;
  while(c = *ptr++)
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
  return hash;
}

static struct record* get_record(zip_archive* arc, char* filename){
  int i = hash(filename) % TABLE_SIZE;
  struct record* ptr = arc->table[i];
  while(ptr){
    if(strcmp(filename, ptr->filename)) return ptr;
    ptr = ptr->next;
  }
  return NULL;
}

static void set_record(zip_archive* arc, struct record* r){
	/* check if r is a dir or file */

	/* if file, find dir. insert into dir. insert file */
	/* if dir, find dir or create, insert into parent */
  int i = hash(r->filename) % TABLE_SIZE;
  if(arc->table[i] == NULL)
    arc->table[i] = r;
  else{
    struct record* ptr = arc->table[i];
    while(ptr->next){
      ptr = ptr->next;
    }
    ptr->next = r;
  }
}


static int skip(zip_archive* arc, int count){
	return -1; /* FIXME */
}

static int read_bytes(zip_archive* arc, char* buf, int count){
	return -1;
}

static int read_long(zip_archive* arc, unsigned* n){
  unsigned char b[4];
  int e = arc->read(arc->userdata, b, 4);
  if(e < 0){return -1;}
  *n = b[0] | (b[1]<<8) | (b[2]<<16) | (b[3]<<24);
  return 0;
}

static int read_short(zip_archive* arc, unsigned* n){
  unsigned char b[2];
  int e = arc->read(arc->userdata, b, 2);
  if(e < 0){return -1;}
  *n = b[0] | (b[1]<<8);
  return 0;
}


static struct record* parse_local_header(zip_archive* arc){
	unsigned L1, L2;
	struct record* r = malloc(sizeof(struct record));
	unsigned n;
	unsigned ddesc;

	if(skip(arc, 4) ||
	read_short(arc, &r->method) ||
	skip(arc, 8) ||
	read_long(arc, &n) ||
	read_long(arc, &n) ||
	read_short(arc, &L1) ||
	read_short(arc, &L2)){
		free(r); 
		return NULL;
	}

	char* filename = malloc(L1+1);
	if(read_bytes(arc, filename, L1) ||
	skip(arc, L2) ||
	skip(arc, r->clen)){
		free(filename);
		free(r);
		return NULL;
	}
	r->filename = filename;
	r->offset = arc->ptr + 30 + L1 + L2;
	arc->ptr += 26 + L1 + L2 + r->clen + ddesc;

	return r;
}

static int build_directory(zip_archive* arc){
  /* for each local file header
     create a record, fill in the info
     if filename ends in /, its a directory
     else create two copies of the record
       and place one in the directory record contents */

	while(1){
		struct record* r = parse_local_header(arc);
		if(r == NULL){ /* end of file chunks */
			break;
		}
		set_record(arc, r);
	}


	return 0;
}


static void free_directory_r(struct record* r){
  if(r->contents) free_directory_r(r->contents);
  if(r->next) free_directory_r(r->next);
  free(r);
}

static void free_directory(zip_archive* arc){
	int i;
	for(i=0; i<TABLE_SIZE; i++){
		if(arc->table[i]) free_directory_r(arc->table[i]);
	}  
}


static int fill_inbuf(zip_file* f){
  int n = f->arc->read(f->arc->userdata, f->inbuf, BUF_SIZE);
  if(n<0) return -1;
  f->strm.next_in = f->inbuf;
  f->strm.avail_in = n;
  return n;
}

static int inflate_chunk(zip_file* f, byte buf[], int count){
  f->strm.next_out = buf;
  f->strm.avail_out = count;
  int e = inflate(&f->strm, Z_SYNC_FLUSH);
  if(e==Z_STREAM_END){
    f->eof = 1;
    return count - f->strm.avail_out;
  }
  if(e==Z_OK){
    return count - f->strm.avail_out;
  }
  return e;
}



static int file_read(void* f, byte buf[], int count){
  return fread(buf, 1, count, f);
}

static int file_seek(void* f, int offset, int whence){
  return fseek(f, offset, whence);
}

static void file_close(void* f){
  fclose(f);
}



/* public functions */

zip_archive* zip_aropenf(char* filename){
  FILE* f = fopen(filename, "r");
  if(f == NULL){
    /* error */
    return NULL;
  }
  zip_reader rd = {file_read, file_seek, file_close, f};
  return zip_aropen(&rd);
}

zip_archive* zip_aropen(zip_reader* rd){
  zip_archive* arc = malloc(sizeof(zip_archive));
  if(arc == NULL){
    /* out of memory */
    return NULL;
  }
  arc->read = rd->read;
  arc->seek = rd->seek;
  arc->close = rd->close;
  arc->userdata = rd->userdata;

  /*TODO: build a directory structure*/

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
  zip_file* f = malloc(sizeof(zip_file));
  if(f == NULL){
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

  /*TODO: get file offset and location in arc */
  f->len = 0;
  f->ptr = 0;

  return f;
}

void zip_fclose(zip_file* f){
  free(f);
}

int zip_fread(zip_file* f, byte buf[], int count){
  int total = 0;
  int e, n;

  while(count > 0 && !f->eof){
    e = fill_inbuf(f);
    n = inflate_chunk(f, buf, count);
    count -= n;
    total += n;
  }

  return total;
}

int zip_feof(zip_file* f){
  return f->eof;
}




zip_dir* zip_opendir(zip_archive* arc, char* path){
  zip_dir* dir = malloc(sizeof(zip_dir));
  if(dir == NULL){
    /* out of memory */
    return NULL;
  }

  struct record* r = get_record(arc, path);
  if(r == NULL){
    /* file not found */
    return NULL;
  }

  dir->ptr = r->contents;
  return dir;
}

char* zip_readdir(zip_dir* dir){
  if(dir->ptr == NULL) return NULL;
  char* filename = dir->ptr->filename;
  dir->ptr = dir->ptr->next;
  return filename;
}

void zip_closedir(zip_dir* dir){
  free(dir);
}
