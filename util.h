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





typedef unsigned long utf32;

typedef int rng_state;
rng_state pseed(int s);
int prand(rng_state* x);
int prandi(rng_state* x, int a, int b);
double prandr(rng_state* x, double a, double b);
int gcd(int u, int v);

int randint(int a, int b);
double randf();

int unicode_getc(char* str, utf32* u);


typedef struct treenode treenode;
struct treenode {
  treenode* l;
  treenode* r;
  void* key;
  void* value;
};

void tree_insert(treenode* root, 
                 int (*compare)(void* k1, void* k2), 
                 void* key, void* value);

void* tree_search(treenode* root,
                  int (*compare)(void* k1, void* k2),
                  void* key);


