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

/* error routines */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

void report_error(const char* format, ...){
  va_list ap;
  va_start(ap, format);

  vprintf(format, ap);

  va_end(ap);
}

void fatal_error(const char* format, ...){
  va_list ap;
  va_start(ap, format);
  //report_error(format, ap);
  vprintf(format, ap);
  va_end(ap);
  exit(-1);
}

void* xmalloc(size_t size){
  void* v = malloc(size);
  if(!v){
    out_of_memory("xmalloc");
  }
  return v;
}

char* strxcpy(const char* str){
  char* cpy = xmalloc(strlen(str)+1);
  strcpy(cpy, str);
  return cpy;
}


void strmcat(char* dst, const char* src, size_t n){
  strncat(dst, src, n - strlen(src) - 1);
}


void out_of_memory(const char* prefix){
  report_error("%s: out of memory", prefix);
  exit(-2);
}