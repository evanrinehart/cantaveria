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
#include <string.h>

#include <util.h>
#include <list.h>
#include <loader.h>
#include <graphics.h>

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
	int i, j;
	int w, h;
	int flags;
} water;

typedef struct {
	int zid;
	int i, j;
	char fg[20][15];
	char bg[20][15];
	list* decs;
	list* waters;
	char snap[4];
	zoneport* exit;
} stage;

typedef struct {
	int id;
	char name[32];
	int w, h, i, j;
	int fgtiles;
	int bgtiles;
	int dectiles;
	int bgimage;
	enum tile_shape shapes[256];
	stage** stages; // w by h array
} zone;

/*
how the above is stored in a binary file
string    z->name
byte[256] z->shapes
short[4]  z->{i, j, w, h}
short     [number of stages N]
[N of the following
	short[2]  s->{i, j}
	byte      unknown
	string[4] exits
	byte[20x15] s->fg
]

new version
byte[4]   0x0a 0x0b 0x0c 0x0d
int       format version (1)
int	  z->id
short[4]  z->{i, j, w, h}
string    fgtiles file
string    bgtiles file
string    dectiles file
string    bgimage file
byte[256] z->shapes
short     number of stages N
[N of the following
	short[2]  s->{i, j}
	byte[4]   s->snap
	byte[4]   'exit' or 'none'
		a zone port is next if previous was 'exit'
		string   zone
		short[2] i, j
		byte     'L' 'U' 'D' or 'R'
	short     number of decorations M
	[M of the following
		short[2] i, j
		short    tile
	]
	short    number of waters W
	[W of the following
		short[4] i, j, w, h
		int flags
	]
	byte[20x15] fg
	byte[20x15] bg
]
*/

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






/******** stage loading routines ********/
static int load_stage_dimensions(stage* s, reader* rd){
	return
		read_short(rd, &s->i) ||
		read_short(rd, &s->j);
}

static int load_stage_snap(stage* s, reader* rd){
	return read_bytes(rd, (unsigned char*)s->snap, 4);
}

static int load_stage_exit(stage* s, reader* rd){
	char what[5] = {0,0,0,0,0};
	if(read_bytes(rd, (unsigned char*)what, 4)) return -1;

	if(strcmp(what, "none") == 0){
		return 0;
	}
	if(strcmp(what, "exit") == 0){
/* FIXME */
printf("INCOMPLETE load_stage_exit doesnt work yet\n");
		return -1;
	}

	error_msg("load_zone: invalid stage header (missing exit)\n");
	return -1;
}

static int load_stage_decorations(stage* s, reader* rd){
	int N;
	if(read_short(rd, &N)) return -1;

	if(N > 0){
/* FIXME */
printf("INCOMPLETE load_stage_decorations doesnt work yet\n");
	return -1;
	}

	return 0;
}

static int load_stage_waters(stage* s, reader* rd){
	int N;
	if(read_short(rd, &N)) return -1;

	if(N > 0){
/* FIXME */
printf("INCOMPLETE load_stage_waters doesnt work yet\n");
	return -1;
	}

	return 0;
}

static int load_stage_data(stage* s, reader* rd){
	int i, j;
	for(i=0; i<15; i++){
		for(j=0; j<20; j++){
			if(read_byte(rd, (int*)&s->fg[i][j])) return -1;
		}
	}
	for(i=0; i<15; i++){
		for(j=0; j<20; j++){
			if(read_byte(rd, (int*)&s->bg[i][j])) return -1;
		}
	}
	return 0;
}

static stage* load_stage(reader* rd){
	stage* s = xmalloc(sizeof(stage));
	s->decs = empty();
	s->exit = NULL;
	s->snap[NORTH] = 0;
	s->snap[WEST] = 0;
	s->snap[EAST] = 0;
	s->snap[SOUTH] = 0;

	if(
		load_stage_dimensions(s, rd) ||
		load_stage_snap(s, rd) ||
		load_stage_exit(s, rd) ||
		load_stage_decorations(s, rd) ||
		load_stage_waters(s, rd) ||
		load_stage_data(s, rd)
	)
	{
		recycle(s->decs);
		free(s);
		return NULL;
	}
	return s;
}

static int load_zone_bitmap(reader* rd, int* out){
	char* filename;
	if(read_string(rd, &filename)){
		error_msg("load_zone: read error (graphics list)\n");
		return -1;
	}

	if(strlen(filename) == 0){
		error_msg("load_zone: no graphics file specified\n");
		return -1;
	}

	*out = load_bitmap(filename);
	free(filename);
	if(*out < 0){
		error_msg("load_zone: can't load graphics %s\n", filename);
		return -1;
	}
	else{
		return 0;
	}
}

static int load_zone_gfx(zone* z, reader* rd){
	return
		load_zone_bitmap(rd, &z->fgtiles) ||
		load_zone_bitmap(rd, &z->bgtiles) ||
		load_zone_bitmap(rd, &z->dectiles) ||
		load_zone_bitmap(rd, &z->bgimage);
}

static int load_zone_shapes(zone* z, reader* rd){
	int i;
	for(i=0; i<256; i++){
		if(read_byte(rd, (int*)&z->shapes[i])){
			return -1;
		}
	}
	return 0;
}

static int load_zone_dimensions(zone* z, reader* rd){
	return
		read_short(rd, &z->i) ||
		read_short(rd, &z->j) ||
		read_short(rd, &z->w) ||
		read_short(rd, &z->h);
}

static int load_zone_stages(zone* z, reader* rd){
	int i, j, N;

	z->stages = xmalloc(z->w * z->h * sizeof(stage*));

	for(i=0; i < z->w; i++){
		for(j=0; j < z->h; j++){
			*(z->stages + i + j*z->w) = NULL;
		}
	}

	if(read_short(rd, &N)) return -1;

printf("number of stages %d\n", N);
	for(i=0; i<N; i++){
		stage* s = load_stage(rd);
		if(s == NULL) return -1;
		z->stages[s->i + s->j * z->w] = s;
	}

	return 0;
}

int load_zone_header(zone* z, reader* rd){
	unsigned char magic1[4];
	unsigned char magic2[4] = {0x0a, 0x0b, 0x0c, 0x0d};
	int format;

	if(
		read_bytes(rd, magic1, 4) ||
		read_int(rd, &format) ||
		read_int(rd, &z->id)
	)
		return -1;

	if(memcmp(magic1, magic2, 4) != 0){
		error_msg("load_zone: wrong file type (missing magic)\n");
		return -1;
	}

	if(format != 1){
		error_msg("load_zone: wrong format version\n");
		return -1;
	}

	return 0;
}


int load_zone(char* filename){
	reader* rd = data_open("zones/", filename);
	if(rd == NULL){
		error_msg("load_zone: error opening \"%s\"\n", filename);
		return -1;
	}

	zone* z = xmalloc(sizeof(zone));
	strncpy(z->name, filename, 32);
	z->id = 0; /* supposed to be in file */
	z->fgtiles = -1;
	z->bgtiles = -1;
	z->dectiles = -1;
	z->bgimage = -1;

	if(
		load_zone_header(z, rd) ||
		load_zone_dimensions(z, rd) ||
		load_zone_gfx(z, rd) ||
		load_zone_shapes(z, rd) ||
		load_zone_stages(z, rd)
	){
		error_msg("load_zone: error reading \"%s\"\n", filename);
		free(z);
		loader_close(rd);
		return -1;
	}

	push(zones, z);
	return z->id;
}
/**************************************/





void print_shapes(enum tile_shape shapes[256]){
	int i, j;
	for(i=0; i<16; i++){
		printf("  ");
		for(j=0; j<16; j++){
			printf("%02x ", shapes[i*16 + j]);
		}
		printf("\n");
	}
}

void print_dir(char snap[4], enum cardinal dir, char* c){
	if(snap[dir]) printf("%s", c);
	else printf(" ");
}

void print_snaps(char snap[4]){
	printf("    snap: (");
	print_dir(snap, WEST, "L");
	print_dir(snap, NORTH, "U");
	print_dir(snap, SOUTH, "D");
	print_dir(snap, EAST, "R");
	printf(")\n");
}

void print_stages(stage** stages, int w, int h){
	int i, j;
	for(j=0; j<h; j++){
		printf("  ");
		for(i=0; i<w; i++){
			stage* s = stages[j*w + i];
			if(s == NULL){
				printf(" ");
			}
			else{
				printf("O");
			}
		}
		printf("\n");
	}

	for(j=0; j<h; j++){
		for(i=0; i<w; i++){
			stage* s = stages[j*w + i];
			if(s == NULL) continue;

			printf("  stage (%d, %d):\n", i, j);
			printf("    decs: %d\n", length(s->decs));
			print_snaps(s->snap);
			printf("    exit: %p\n", s->exit);
			printf("\n");
		}
	}
}


void print_zone(int id){
	zone* z = find_zone_by_id(id);
	printf("zone: %s\n", z->name);

	printf("  id: %d\n", z->id);
	printf("  dimensions: %d x %d\n", z->w, z->h);
	printf("  position: (%d, %d)\n", z->i, z->j);
	printf("  fg: %d\n", z->fgtiles);
	printf("  bg: %d\n", z->bgtiles);
	printf("  dec: %d\n", z->dectiles);
	printf("  bgimage: %d\n", z->bgimage);
	printf("  shapes:\n");
	print_shapes(z->shapes);
	printf("  stages:\n");
	print_stages(z->stages, z->w, z->h);
}



void stage_bind_camera(location l, int* x, int* y){
	//for each snap, snap x y
}

void draw_stage_bg(location loc, int cx, int cy){
	//draw background
	//draw bg tiles
}

void draw_stage_fg(location loc, int cx, int cy){
	//draw water
	//fg tiles
	//draw decorations
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




void stage_init(){
	zones = empty();
}
