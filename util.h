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

void report_error(const char* format, ...);
void fatal_error(const char* format, ...);
void* xmalloc(size_t size);
char* strxcpy(const char* str);
void strmcat(char* dst, const char* src, size_t n);
void out_of_memory(const char*);

//typedef struct {
//  unsigned char b[4];
 // unsigned long n;
//} utf32;
typedef unsigned long utf32;

int unicode_getc(char* str, utf32* u);
