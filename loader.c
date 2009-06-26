/*
   Cantaveria - action adventure platform game
   Copyright (C) 2009  Evan Rinehart

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



#include "loader.h"
#include "util.h"

struct reader {
  ZZIP_FILE* f;
  int next_c;
};

ZZIP_DIR* zzip_dir;
int errno;

void loader_init(char* filename){
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

reader* loader_open(char* filename){
  char buf[1024] = "data/";
  int L = strlen(buf);
  strncpy(buf+L,filename,1024-L);
  buf[1023] = 0;

  reader* rd = xmalloc(sizeof(reader));

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
  //return zzip_file_read(rd->f, buf, count);
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
      loader_read(rd, &c, 1);
    }

    /* see if it is a end of line sequence */
    if(c=='\r'){
      loader_read(rd, &c, 1);
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

//printf("loader_scanline: %s\n",buf);

  va_list ap;
  va_start(ap, format);

  int ret = vsscanf(buf,format,ap);

  va_end(ap);

  return ret;
}