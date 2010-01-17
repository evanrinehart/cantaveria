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


#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "rng.h"
#include "util.h"

/* error reporting */
void report_verror(const char* format, va_list ap){
	vprintf(format, ap);
}

void report_error(const char* format, ...){
	va_list ap;
	va_start(ap, format);
	report_verror(format, ap);
	va_end(ap);
}

void fatal_error(const char* format, ...){
	va_list ap;
	va_start(ap, format);
	report_verror(format, ap);
	va_end(ap);
	exit(-1);
}

void out_of_memory(const char* prefix){
	report_error("%s: *out of memory*\n", prefix);
	exit(-2);
}

/* 'safe' functions */
void* xmalloc(size_t size){
	void* v = malloc(size);
	if(!v){
		/* On Linux malloc by default does not return NULL
		even if there is no more memory. So this function
		isn't really 'safe' on that system. */
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




/*
   decodes utf8 encoded string str and places the
   next character in u.
   returns the number of bytes of str consumed.
*/
int unicode_getc(char* str, utf32* u){
	/*unsigned char b[4] = {str[0], str[1], str[2], str[3]};*/
	unsigned char b[4];
	unsigned char a[4] = {0,0,0,0};
	int N;

	memcpy(b, str, 4);

	/* 1111 0xf
	   1110 0xe
	   1100 0xc
	   1000 0x8 */

	if((b[0] & 0x80) == 0){ /*one byte sequence*/
		a[3] = b[0];
		N = 1;
	}

	else if(
		((b[0]&0xe0)==0xc0) &&
		((b[1]&0xc0)==0x80)
	){
		/*two byte sequence*/
		a[3] = ((b[0]&0x03)<<6)|(b[1]&0x3f);
		a[2] = (b[0]&0x1c)>>2;
		N = 2;
	}

	else if(
		((b[0]&0xf0)==0xe0) &&
		((b[1]&0xc0)==0x80) &&
		((b[2]&0xc0)==0x80)
	){
		/*three byte sequence*/
		a[3] = ((b[1]&0x03)<<6)|(b[2]&0x3f);
		a[2] = ((b[0]&0x0f)<<4)|((b[1]&0x3c)>>2);
		N = 3;
	}

	else if(
		((b[0]&0xf8)==0xf0) &&
		((b[1]&0xc0)==0x80) &&
		((b[2]&0xc0)==0x80) &&
		((b[3]&0xc0)==0x80)
	){
		/*four byte sequence*/
		a[3] = ((b[2]&0x03)<<6)|(b[3]&0x3f);
		a[2] = ((b[1]&0x0f)<<4)|((b[2]&0x3c)>>2);
		a[1] = ((b[0]&0x03)<<2)|((b[1]&0x30)>>4);
		N = 4;
	}

	else {
		a[3] = '?';
		N = 4;/*FIXME find next valid byte*/
	}

	*u = (a[0]<<24) | (a[1]<<16) | (a[2]<<8) | a[3];
	return N;
}



void tree_insert(
	struct treenode* root,
	int (*compare)(void* k1, void* k2),
	void* key, void* value)
{

	struct treenode* node = xmalloc(sizeof(struct treenode));
	struct treenode* ptr;
	node->key = key;
	node->value = value;
	node->l = NULL;
	node->r = NULL;

	ptr = root;
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

void* tree_search(
	struct treenode* root,
	int (*compare)(void* k1, void* k2),
	void* key)
{
	if(root==NULL){
		return NULL;
	}
	else if(compare(root->key, key)>0){
		return tree_search(root->r, compare, key);
	}
	else if(compare(root->key, key)<0){
		return tree_search(root->l, compare, key);
	}
	else{
		return root->value;
	}
}



/* rng utility functions */
void rand_reset(unsigned s){
	zsrand(s);
}

int randi(int a, int b){
	int L = b-a;
	int P = 1;
	int x;

	while(P < L){
		P <<= 1;
	}

	do{
		x = zrand() & (P-1);
	} while(x > L);

	return x+a;
}

double randf(){
	double D = UINT_MAX;
	unsigned x = zrand();
	return x / D;
}


/* Copyright 300BC Euclid */
int gcd(int a, int b){
	while(b){
		int t = b;
		b = a % b;
		a = t;
	}
	return a;
}
