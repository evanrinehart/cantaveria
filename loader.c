/*
   Cantaveria - action adventure platform game
   Copyright (C) 2009 2010 Evan Rinehart

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to

   The Free Software Foundation, Inc.
   51 Franklin Street, Fifth Floor
   Boston, MA  02110-1301, USA
*/

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>


#include <loader.h>
#include <util.h>
#include <zip.h>

struct reader {
	zip_file* f;
	int next_c;
};

zip_archive* arc;


void loader_init(){
	char* filename = "data.zip";
	arc = zip_aropenf(filename);
	if(arc == NULL){
		fatal_error("loader: unable to load data archive \"%s\" (%s)\n", filename, zip_geterror());
	}
	boot_msg("loader: ... OK\n");
}

void loader_quit(){
	zip_arclose(arc);
}

reader* data_open(char* dir, char* filename){
	char buf[1024];
	strcpy(buf, dir);
	strcat(buf, filename);
	return loader_open(buf);
}

reader* loader_open(char* filename){
	char buf[1024] = "data/";
	int L = strlen(buf);
	strncpy(buf+L,filename,1024-L);
	buf[1023] = 0;

	reader* rd = xmalloc(sizeof(reader));
	rd->f = zip_fopen(arc, filename);
	if(!rd->f){
		error_msg("loader: can't open %s (%s)\n", filename, zip_geterror());
		free(rd);
		return NULL;
	}
	return rd;
}


void loader_close(reader* rd){
	zip_fclose(rd->f);
	free(rd);
}


int loader_read(reader* rd, void* buf, int count){
	return zip_fread(rd->f, buf, count);
}

unsigned char* loader_readall(char* filename, int* size){
	reader* rd = loader_open(filename);
	if(!rd) return NULL;

	/* somehow read all of rd into a buffer and return it */

	return NULL;
}

int loader_readline(reader* rd, char* buf, int size){
	char c;
	int i = 0;
	int n;

	while(i < size){
		n = loader_read(rd, &c, 1);
		if(n == 0){ /* end of file */
			buf[i] = '\0';
			return 0;
		}

		if(n < 0){
			error_msg("loader_readline: %s\n", zip_geterror());
			return -1;
		}

		if(c == '\r'){ /* CRLF ? */
			n = loader_read(rd, &c, 1);
			if(n == 0){ /* file ended with CR... well take it */
				buf[i] = '\0';
				return 0;
			}

			if(n < 0){
				error_msg("loader_readline: %s\n", zip_geterror());
				return -1;
			}

			if(c != '\n'){
				error_msg("loader_readline: I cannot read lines ending in CR and not CRLF\n");
				return -1;
			}

			buf[i] = '\0';
			return 0;
		}

		if(c == '\n'){ /* LF */
			buf[i] = '\0';
			return 0;
		}

		buf[i] = c;
		i += 1;
	}

	error_msg("loader_readline: buffer size too small\n");
	return 0;
}

int loader_scanline(reader* rd, char* format, ...){
	char buf[256] = "";
	va_list ap;
	int ret;

	if(loader_readline(rd, buf, 256) < 0){
		return -1;
	}

	va_start(ap, format);
	ret = vsscanf(buf,format,ap);
	va_end(ap);

	return ret;
}




/*binary i/o*/
int read_byte(reader* rd){
	unsigned char c = 0;
	loader_read(rd, &c, 1);
	return c;
}

int read_short(reader* rd){
	unsigned char c[2] = {0,0};
	loader_read(rd, c+0, 1);
	loader_read(rd, c+1, 1);
	return (c[0]<<8) | c[1];
}

int read_int(reader* rd){
	unsigned char c[4] = {0,0,0,0};
	loader_read(rd, c+0, 1);
	loader_read(rd, c+1, 1);
	loader_read(rd, c+2, 1);
	loader_read(rd, c+3, 1);
	return (c[0]<<24) | (c[1]<<16) | (c[2]<<8) | c[3];
}

char* read_string(reader* rd){
	unsigned int L = read_int(rd);
	if(L==0) return NULL;
	char* S = xmalloc(L+1);
	S[L] = '\0';
	loader_read(rd, S, L);
	return S;
}



/*
list* loader_readdir(char* path){
	zip_dir* dir = zip_opendir(arc, path);
	char* entry = zip_readdir(dir);
	list* dirs = empty();

	while(entry){
		push(dirs, entry);
		entry = zip_readdir(dir);
	}

	return dirs;
}

void loader_freedirlist(list* dirs){
	list* ptr = dirs->next;
	while(ptr){
		free(ptr->item);
		ptr = ptr->next;
	}
	list_recycle(dirs);
}
*/
