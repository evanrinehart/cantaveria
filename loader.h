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

typedef struct reader reader;

void loader_init();
void loader_quit();

reader* data_open(char* dir, char* filename);
reader* loader_open(char* filename);

int loader_read(reader* rd, void* buf, int count);
int loader_scanline(reader* rd, char* format, ...);
unsigned char* loader_readall(char* filename, int* size);
void loader_close(reader* rd);

char** loader_readdir(char* path);
void loader_freedirlist(char** list);

/*binary i/o*/
int read_byte(reader* rd);
int read_short(reader* rd);
int read_int(reader* rd);
char* read_string(reader* rd);
