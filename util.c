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
  report_error("%s: out of memory\n", prefix);
  exit(-2);
}



/*
decodes utf8 encoded string str and places the
next character in u.
returns the number of bytes of str consumed.
*/
int unicode_getc(char* str, utf32* u){
  unsigned char b[4] = {str[0], str[1], str[2], str[3]};
  unsigned char a[4] = {0,0,0,0};
  int N;

  /* 1111 0xf
     1110 0xe
     1100 0xc
     1000 0x8 */

  if((b[0] & 0x80) == 0){/*one byte sequence*/
    a[3] = b[0];
    N = 1;
  }
  else if(((b[0]&0xe0)==0xc0) &&
          ((b[1]&0xc0)==0x80) ){/*two byte sequence*/
    a[3] = ((b[0]&0x03)<<6)|(b[1]&0x3f);
    a[2] = (b[0]&0x1c)>>2;
    N = 2;
  }
  else if(((b[0]&0xf0)==0xe0) &&
          ((b[1]&0xc0)==0x80) &&
          ((b[2]&0xc0)==0x80) ){/*three byte sequence*/
    a[3] = ((b[1]&0x03)<<6)|(b[2]&0x3f);
    a[2] = ((b[0]&0x0f)<<4)|((b[1]&0x3c)>>2);
    N = 3;
  }
  else if(((b[0]&0xf8)==0xf0) &&
          ((b[1]&0xc0)==0x80) &&
          ((b[2]&0xc0)==0x80) &&
          ((b[3]&0xc0)==0x80) ){/*four byte sequence*/
    a[3] =((b[2]&0x03)<<6)|(b[3]&0x3f);
    a[2] =((b[1]&0x0f)<<4)|((b[2]&0x3c)>>2);
    a[1] =((b[0]&0x03)<<2)|((b[1]&0x30)>>4);
    N = 4;
  }
  else {
    a[3] = '?';
    N = 4;/*FIXME find next valid byte*/
  }
  *u = (a[0]<<24) | (a[1]<<16) | (a[2]<<8) | a[3];
  return N;
}



void tree_insert(struct treenode* root,
                 int (*compare)(void* k1, void* k2), 
                 void* key, void* value){
  struct treenode* node = xmalloc(sizeof(struct treenode));
  node->key = key;
  node->value = value;
  node->l = NULL;
  node->r = NULL;

  struct treenode* ptr = root;
  while(1){
    if( compare(ptr->key, key) < 0 ){
      if(ptr->l){ptr = ptr->l;}
      else{ptr->l = node; break;}
    }
    else if( compare(ptr->key, key) > 0){
      if(ptr->r){ptr = ptr->r;}
      else{ptr->r = node; break;}
    }
    else{ /* key already exists */
      report_error("tree_insert: key already exists\n");
      break;
    }
  }
}

void* tree_search(struct treenode* root,
                  int (*compare)(void* k1, void* k2),
                  void* key){
  if(root==NULL){
    return NULL;
  }
  else if(compare(root->key, key)>0){
//printf("%p -> go right\n",root->key);
    return tree_search(root->r, compare, key);
  }
  else if(compare(root->key, key)<0){
//printf("%p -> go left\n",root->key);
    return tree_search(root->l, compare, key);
  }
  else{
//printf("%p -> found\n",key);
    return root->value;
  }
}



int randint(int a, int b){
  int L = b-a+1;
  return (rand()%L)+a;
}


#define PRAND_MAX 65537

rng_state pseed(int s){
  rng_state x = 2;
  for(int i=0; i<s; i++){
    prand(&x);
  }
  return x;
}

int prand(rng_state* x){
  int G = 75;
  *x = (*x * G) % PRAND_MAX;
  return *x;
}

int prandi(rng_state* x, int a, int b){
  int L = b-a+1;
  return (prand(x)%L)+a;
}

double prandr(rng_state* x, double a, double b){
  double R = prand(x)/((double)PRAND_MAX);
  double L = b-a;
  return (R*L)+a;
}



/* stole this from wikipedia */
unsigned gcd(unsigned u, unsigned v)
{
     int shift;
 
     /* GCD(0,x) := x */
     if (u == 0 || v == 0)
       return u | v;
 
     /* Let shift := lg K, where K is the greatest power of 2
        dividing both u and v. */
     for (shift = 0; ((u | v) & 1) == 0; ++shift) {
         u >>= 1;
         v >>= 1;
     }
 
     while ((u & 1) == 0)
       u >>= 1;
 
     /* From here on, u is always odd. */
     do {
         while ((v & 1) == 0)  /* Loop X */
           v >>= 1;
 
         /* Now u and v are both odd, so diff(u, v) is even.
            Let u = min(u, v), v = diff(u, v)/2. */
         if (u < v) {
             v -= u;
         } else {
             unsigned int diff = u - v;
             u = v;
             v = diff;
         }
         v >>= 1;
     } while (v != 0);
 
     return u << shift;
 }