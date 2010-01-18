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

#include <zzip/lib.h>



#include <loader.h>
#include <util.h>

struct reader {
	ZZIP_FILE* f;
	int next_c;
};

ZZIP_DIR* zzip_dir;
int errno;

void loader_init(){
	//zzip_dir = zzip_dir_open(filename, 0);
	//if(!zzip_dir){
	//  report_error("loader: unable to open game data in %s (%s)\n",
	//               filename, strerror( errno ) );
	//  exit(-1);
	// }
}

void loader_quit(){
	//zzip_dir_close(zzip_dir);
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
	//printf("loader: %s\n",buf);
	rd->next_c = -1;
	//rd->f = zzip_file_open(zzip_dir, buf, 0);
	rd->f = zzip_open(buf, 0);
	if(!rd->f){
		//report_error("loader: unable to open %s (%s)\n",
		//           filename, zzip_strerror_of( zzip_dir ) );
		report_error("loader: unable to open %s (%s)\n",
				filename, strerror( errno ) );
		free(rd);
		return NULL;
	}
	return rd;
}


void loader_close(reader* rd){
	//zzip_file_close(rd->f);
	zzip_fclose(rd->f);
	free(rd);
}


int loader_read(reader* rd, void* buf, int count){
	return zzip_read(rd->f, buf, count);
}

unsigned char* loader_readall(char* filename, int* size){
	ZZIP_STAT zs;
	reader* rd = loader_open(filename);
	if(!rd) return NULL;
	if(zzip_fstat(rd->f, &zs) < 0){
		report_error("loader: stat error on %s\n",filename);
		return NULL;
	}
	int N = zs.st_size;
	unsigned char* buf = xmalloc(N);
	loader_read(rd,buf,N);
	if(size) *size = N;
	loader_close(rd);
	return buf;
}


int loader_scanline(reader* rd, char* format, ...){

	char buf[256];
	int i=0;
	while(i<255){
		char c;

		/* get next character */
		if(rd->next_c != -1){
			c = rd->next_c;
			rd->next_c = -1;
		}
		else{
			int n = loader_read(rd, &c, 1);
			if(n==0){
				break;
			}
		}

		/* see if it is a end of line sequence */
		if(c=='\r'){
			int n = loader_read(rd, &c, 1);
			if(n==0){
				break;
			}
			if(c!='\n'){
				rd->next_c = c;
			}
			break;
		}
		else if(c=='\n'){
			break;
		}

		buf[i++] = c;
	}
	buf[i]='\0';

	va_list ap;
	va_start(ap, format);

	int ret = vsscanf(buf,format,ap);

	va_end(ap);

	return ret;
}




/*binary i/o*/
unsigned char read_byte(reader* rd){
	unsigned char c;
	loader_read(rd, &c, 1);
	return c;
}

short read_short(reader* rd){
	unsigned char c[2];
	loader_read(rd, c+0, 1);
	loader_read(rd, c+1, 1);
	return (c[0]<<8) | c[1];
}

int read_int(reader* rd){
	unsigned char c[4];
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




char** loader_readdir(char* path){
	char buf[1024] = "data/";
	strcat(buf, path);

	ZZIP_DIR* dir = zzip_opendir(buf);

	int N = 0;
	ZZIP_DIRENT* ent;
	while( (ent = zzip_readdir(dir)) ) N++;

	char** res = xmalloc((N+1)*sizeof(char*));

	zzip_closedir(dir);
	dir = zzip_opendir(buf);

	int i = 0;
	while(i < N+1){
		ent = zzip_readdir(dir);
		if(!ent){
			res[i] = NULL;
			i++;
		}
		else if(ent->d_name[0] == '.'){
		}
		else{
			res[i] = xmalloc(strlen(ent->d_name)+1);
			strcpy(res[i], ent->d_name);
			i++;
		}
	}

	return res;
}

void loader_freedirlist(char** list){
	int i;
	for(i=0; list[i]; i++){
		free(list[i]);
	}
}

