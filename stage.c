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


#include <stdio.h>
#include <stdlib.h>

#include <util.h>
#include <list.h>

#include <stage.h>

enum cardinal {NORTH, SOUTH, EAST, WEST};


enum tile_shape {
SHAPE_FREE,
SHAPE_SQUARE,
SHAPE_NULL,

SHAPE_TRI_NE,
SHAPE_TRI_NW,
SHAPE_TRI_SE,
SHAPE_TRI_SW,

SHAPE_LTRAP_FLOOR,
SHAPE_RTRAP_FLOOR,
SHAPE_LSLOPE_FLOOR,
SHAPE_RSLOPE_FLOOR,
SHAPE_LTRAP_CEIL,
SHAPE_RTRAP_CEIL,
SHAPE_LSLOPE_CEIL,
SHAPE_RSLOPE_CEIL,

SHAPE_HALF_FLOOR,
SHAPE_HALF_CEIL
};

typedef struct {
	int tile;
	int i, j;
} decoration;

typedef struct {
	int zone;
	int i, j;
	enum cardinal dir;
} zoneport;

typedef struct {
	int zid;
	int i, j;
	char fg[20][15];
	char bg[20][15];
	list* decs;
	enum cardinal snap[4];
	zoneport* exit;
} stage;

typedef struct {
	int id;
	int w, h, i, j;
	int fgtiles;
	int bgtiles;
	int dectiles;
	enum tile_shape shapes[256];
	stage** stages; // w by h array
} zone;




typedef struct {
	int x, y;
	int shape;
	int none;
} block;




/* VVV */
list* zones = NULL;
/* ^^^ */



enum cardinal dir_of(int i, int j){
	if(i < 0) return WEST;
	if(j < 0) return NORTH;
	if(i > 0) return EAST;
	if(j > 0) return SOUTH;
	fatal_error("stage.c: (%d,%d) is not a valid direction\n", i, j);
	return NORTH;
}





/* zones and stages */
zone* find_zone_by_id(int id){
	list* ptr = zones->next;
	while(ptr){
		zone* z = ptr->item;
		if(z->id == id) return z;
		ptr = ptr->next;
	}
	return NULL;
}

zone* zone_of_stage(stage* s){
	return find_zone_by_id(s->zid);
}

stage* stage_lookup(zone* z, int i, int j){
	if(i < 0 || i >= z->w || j < 0 || j >= z->h){
		return NULL;
	}
	return *(z->stages + i + j*z->w);
}


stage* follow_zoneport(zoneport* zp){
	zone* z = find_zone_by_id(zp->zone);
	return stage_lookup(z, zp->i, zp->j);
}

stage* find_stage(stage* home, int i, int j){
	zone* z = zone_of_stage(home);
	int si = i/20;
	int sj = j/15;
	int di = home->i - si;
	int dj = home->j - sj;
	if(di==0 && dj==0)
		return home;

	else if(home->exit && home->exit->dir == dir_of(di,dj))
		return follow_zoneport(home->exit);

	else
		return stage_lookup(z, si - z->w, sj - z->h);
}


/* collision computations */
/*FIXME*/
int left_side_of(block bl, int top, int bottom){
	return bl.x;
}

int right_side_of(block bl, int top, int bottom){
	return bl.x + 16;
}

int top_of(block bl, int left, int right){
	return bl.y;
}

int bottom_of(block bl, int left, int right){
	return bl.y + 16;
}

int xhit(int v, block block, int top, int bottom){
	if(v > 0) return left_side_of(block, top, bottom);
	if(v < 0) return right_side_of(block, top, bottom);
	return 0;
}

int yhit(int v, block block, int left, int right){
	if(v > 0) return top_of(block, left, right);
	if(v < 0) return bottom_of(block, left, right);
	return 0;
}

int rect_is_on(block bl, int L, int R, int T, int B){
	int l = bl.x;
	int r = bl.x + 16;
	int t = bl.y;
	int b = bl.y + 16;
	return !(r < L || l > R || t > B || b < T);
}





/* blocks */
block make_block(int shape, int x, int y, int none){
	block bl;
	bl.x = x;
	bl.y = y;
	bl.none = none;
	bl.shape = shape;
	return bl;
}

block no_block(){
	return make_block(0, 0, 0, 1);
}

block null_block(){
	return make_block(SHAPE_NULL, 0, 0, 0);
}

block block_of(int shape, int x, int y){
	return make_block(shape, x, y, 0);
}

/* get block in s at i j */
block block_from_stage(stage* s, int i, int j){
	zone* z = zone_of_stage(s);
	int tile = s->fg[i][j];
	int shape = z->shapes[tile];
	return block_of(shape, i, j);
}

/* get block at absolute x y if you are in stage home */
block get_block(stage* home, int x, int y){
	zone* z = zone_of_stage(home);
	int i = x/1024/16 - (home->i + z->i)*20;
	int j = y/1024/16 - (home->j + z->j)*15;

	stage* s = find_stage(home, i, j);
	if(s == NULL)
		return null_block();
	else
		return block_from_stage(s, i, j);
}

/* returns either a block that the rect is intersecting
   or it returns a block with none set to 1 */
block intersecting_block(stage* s, int left, int right, int top, int bottom){
	int i;
	for(i=0; i<4; i++){
		block bl;
		switch(i){
			case 0: bl = get_block(s, top, left); break;
			case 1: bl = get_block(s, top, right); break;
			case 2: bl = get_block(s, bottom, left); break;
			case 3: bl = get_block(s, bottom, right); break;
		}
		if(rect_is_on(bl, left, right, top, bottom)) return bl;
	}
	return no_block();
}






/* public methods */
void stage_init(){
	zones = empty();
}

int load_zone(const char* filename){
	int w = 4;
	int h = 4;
	int i, j;
	zone* z = xmalloc(sizeof(zone)); /* load routine */

	z->id = 0;
	z->w = w;
	z->h = h;
	z->i = 0;
	z->j = 0;
	z->fgtiles = 0;
	z->bgtiles = 0;
	z->dectiles = 0;
	z->stages = xmalloc(sizeof(stage*)*w*h);

	for(i=0; i<w; i++){
		for(j=0; j<h; j++){
			*(z->stages + i + j*z->w) = NULL;
		}
	}

	for(i=0; i<256; i++){
		z->shapes[i] = 0;
	}

	push(zones, z);
	return z->id;
}


void print_zone(int id){
	zone* z = find_zone_by_id(id);
	printf("zone: %p\n", z);
}


void stage_bind_camera(location l, int* x, int* y){
	//for each snap, snap x y
}

void draw_stage_fg(location loc, int cx, int cy){
	//draw decorations
	//draw water
}

void draw_stage_bg(location loc, int cx, int cy){
	//for each visible tile
	//bg or bg tiles or fg tiles
}

/* returns 1 if a collision will occur, 0 otherwise
   if 1 is returned, x will be set to the collision point */
int stage_xcollide(location loc, int w, int h, int v, int* x){
	zone* z = find_zone_by_id(loc.z);
	stage* s = stage_lookup(z, loc.i, loc.j);
	int X = loc.x + v*10; /* FIXME dt */
	int right = X + w;
	int left = X;
	int top = loc.y;
	int bottom = loc.y + h;

	block bl = intersecting_block(s, left, right, top, bottom);

	if(bl.none) return 0;
	else{
		*x = xhit(v, bl, top, bottom);
		return 1;
	}
}

int stage_ycollide(location loc, int w, int h, int v, int* y){
	return 0;
}
