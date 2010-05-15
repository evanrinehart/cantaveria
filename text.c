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

   evanrinehart@gmail.com
*/

/* messages and text */

#include <stdio.h>
#include <string.h>

#include <list.h>
#include <util.h>
#include <graphics.h>
#include <loader.h>

typedef struct {
  utf32 u;
  int gfx;
  int x, y, w, h;
  int k1, k2;
} vwchar;

struct treenode* chartree = NULL;


int ptrcomp(void* k1, void* k2){
  return (k2 - k1);
}

void set_message(char* str){
  utf32 u;
  int N = unicode_getc(str, &u);
  while(u) {
    vwchar* C = tree_search(chartree, ptrcomp, (void*)u);
    if(C){
/* append this character to the message
   if the current word is too long,
   move the current word to the next line
   if the current word is longer than a whole line
   then just break here (would happen with japanese).
 */
    }
    else{
/*
character not found, so use a rectangle or something
use four tiny numbers to indicate the character
do the same as above
*/
    }
    N += unicode_getc(str+N, &u);
  };
}



void advance_message(){

}

void clear_message(){

}

void complete_message(){

}


void text_init(){
	vwchar* C = xmalloc(sizeof(vwchar));
	C->gfx = 0;
	C->u = ' ';
	C->x = 0;
	C->y = 0;
	C->w = 7;
	C->k1 = 0;
	C->k2 = 0;
	chartree = xmalloc(sizeof(treenode));
	chartree->l = NULL;
	chartree->r = NULL;
	chartree->value = C;
	chartree->key = (void*)' ';

	//set font height


}

vwchar* load_vwchar(reader* rd, int gfx){
	utf32 u;
	int x, y, w, k1, k2;
	char str[256];
	int ret = loader_scanline(rd, "%256s %d %d %d %d %d\n",str,&x,&y,&w,&k1,&k2);
	if(ret == EOF){
		return NULL;
	}
	unicode_getc(str,&u);
	vwchar* C = xmalloc(sizeof(vwchar));
	C->gfx = gfx;
	C->u = u;
	C->x = x;
	C->y = y;
	C->w = w;
	C->k1 = k1;
	C->k2 = k2;
	return C;
}


void print_tree(treenode* node){
	printf("(%lx,",(utf32)node->key);
	if(node->l){
		print_tree(node->l);
	}
	else{
		printf("()");
	}
	printf(",");
	if(node->r){
		print_tree(node->r);
	}
	else{
		printf("()");
	}
	printf(")");
}


void randomly_insert(vwchar* C[], int count){
	int i;
	for(i=0; i<count-1; i++){
		int j = randi(0, count-i-1);
		tree_insert(chartree, ptrcomp, (void*)C[j]->u, C[j]);
		C[j] = C[count-i-1];
		C[count-i-1] = NULL;
	}
}


int load_font(char* path){
	//printf("load_font: loading %s\n",filename);
	reader* rd = loader_open(path);
	if(!rd){
		fatal_error("load_font: cannot open %s\n", path);
	}

	char str[256];
	loader_scanline(rd, "%256s", str);
	int gfx = load_bitmap(str);

	/* we read 64 characters at a time and insert them
	   randomly into the binary search tree. this is supposed
	   to help produce a more balanced tree. */
	vwchar* C[64];
	int ptr = 0;
	int N = 0;
	C[ptr] = load_vwchar(rd, gfx);
	while(C[ptr]){
		N++;
		if(ptr==64){
			randomly_insert(C, 64);
			ptr = 0;
		}
		else{
			C[++ptr] = load_vwchar(rd, gfx);
		}
	}

	randomly_insert(C, ptr);
/*
	printf("  loaded %d characters\n",N);

	printf("  character tree is the following\n");
	print_tree(chartree);
	printf("\n");
*/
	return 0;
}
