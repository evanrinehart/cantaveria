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


/*
stage module is responsible for modelling the static world.
it has three main functions
* load zone files into world
* draw world background and foreground layers
* calculate if and where a moving rectangle will collide with world
*/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <util.h>
#include <list.h>
#include <loader.h>
#include <graphics.h>
#include <video.h>

#include <stage.h>



enum tile_shape {
SHAPE_FREE = '0', /* 'air' */
SHAPE_SQUARE = 'M',

SHAPE_TRI_NE = 'a',
SHAPE_TRI_NW = 'b',
SHAPE_TRI_SE = 'c',
SHAPE_TRI_SW = 'd',

SHAPE_LTRAP_FLOOR = '1', /* trapezoid */
SHAPE_RTRAP_FLOOR = '2',
SHAPE_LSLOPE_FLOOR = '3',
SHAPE_RSLOPE_FLOOR = '4',
SHAPE_LTRAP_CEIL = '5',
SHAPE_RTRAP_CEIL = '6',
SHAPE_LSLOPE_CEIL = '7',
SHAPE_RSLOPE_CEIL = '8',

SHAPE_HALF_FLOOR = 'm',
SHAPE_HALF_CEIL = 'w'
};

typedef struct {
	unsigned char bg;
	unsigned char fg;
	unsigned char shape;
} tile;

typedef struct stage stage;
struct stage {
	char id[256];
	int w, h;
	tile* tiles;
	int bgimage;
	int bgtiles;
	int fgtiles;
	int ox, oy;
	stage* next;
};

char zone_name[256] = "NO ZONE";
stage* stages = NULL;
stage* this_stage = NULL;

/***/


/*
STAGE MODULE

DEADLINES AND MILESTONES

load zones / stages from new format - May 15
display loaded stages - May 22
collision algorithm - May 29

*/



/*** collision stuff ***/

#define UUU(x) (1024*(x))
#define DDD(x) ((x)/1024)

/* does a corner test.
x y is the point of the corner
xp yp is the direction of the rectangle relative to the corner */
static int corner_overlap(int x, int y, int xp, int yp, char shape){
	const int L = UUU(16);
	switch(shape){
		case SHAPE_FREE: return 0;
		case SHAPE_SQUARE: return 1;

		case SHAPE_TRI_NE:
			if(xp<0 && yp>0) return y<x;
			else return 1;
		case SHAPE_TRI_NW:
			if(xp>0 && yp>0) return y<L-x;
			else return 1;
		case SHAPE_TRI_SE:
			if(xp<0 && yp<0) return y>L-x;
			else return 1;
		case SHAPE_TRI_SW:
			if(xp>0 && yp<0) return y>x;
			else return 1;

		case SHAPE_LTRAP_FLOOR:
			if(xp>0 && yp<0) return y>(x/2);
			else return 1;
		case SHAPE_RTRAP_FLOOR:
			if(xp<0 && yp<0) return y>(L/2-x/2);
			else return 1;
		case SHAPE_LSLOPE_FLOOR:
			if(xp>0 && yp<0) return y>(L/2+x/2);
			else if(xp<0 && yp<0) return y>L/2;
			else return 1;
		case SHAPE_RSLOPE_FLOOR:
			if(xp<0 && yp<0) return y>(L-x/2);
			else if(xp>0 && yp<0) return y>L/2;
			else return 1;
		case SHAPE_LTRAP_CEIL:
			if(xp>0 && yp>0) return y<(L-x/2);
			else return 1;
		case SHAPE_RTRAP_CEIL:
			if(xp<0 && yp>0) return y<(L/2+x/2);
			else return 1;
		case SHAPE_LSLOPE_CEIL:
			if(xp>0 && yp>0) return y<(L/2-x/2);
			else if(xp<0 && yp>0) return y<L/2;
			else return 1;
		case SHAPE_RSLOPE_CEIL:
			if(xp<0 && yp>0) return y<(x/2);
			else if(xp>0 && yp>0) return y<L/2;
			else return 1;

		case SHAPE_HALF_FLOOR: return yp > 0 || y > L/2;
		case SHAPE_HALF_CEIL: return yp < 0 || y < L/2;
		default: return 0;
	}
}

/* calculate the collision point given a relevant cross
section and direction of motion. */
static int x_trace(int y, int v, int shape){
	const int L = UUU(16);
	switch(shape){
		case SHAPE_FREE: return 0; /* should not happen */
		case SHAPE_SQUARE:
		case SHAPE_HALF_FLOOR:
		case SHAPE_HALF_CEIL: return v>0 ? 0 : L;
		case SHAPE_TRI_NE: return v>0 ? y : L;
		case SHAPE_TRI_NW: return v>0 ? 0 : L-y;
		case SHAPE_TRI_SE: return v>0 ? L-y : L;
		case SHAPE_TRI_SW: return v>0 ? 0 : y;
		case SHAPE_LTRAP_FLOOR: return v>0 ? 0 : 2*y;
		case SHAPE_RTRAP_FLOOR: return v>0 ? L - 2*y : 0;
		case SHAPE_LSLOPE_FLOOR: return v>0 ? 0 : 2*y - L;
		case SHAPE_RSLOPE_FLOOR: return v>0 ? 2*L -2*y : L;
		case SHAPE_LTRAP_CEIL: return v<0 ? 2*L - 2*y : L;
		case SHAPE_RTRAP_CEIL: return v>0 ? 2*y - L : L;
		case SHAPE_LSLOPE_CEIL: return v>0 ? 0 : L - 2*y;
		case SHAPE_RSLOPE_CEIL: return v>0 ? 2*y : L;
		default: return 0;
	}
}

static int y_trace(int x, int v, int shape){
	const int L = UUU(16);
	switch(shape){
		case SHAPE_FREE: return 0; /* should not happen */
		case SHAPE_SQUARE: return v>0 ? 0 : L;
		case SHAPE_HALF_FLOOR: return v>0 ? L/2 : L;
		case SHAPE_HALF_CEIL: return v>0 ? 0 : L/2;
		case SHAPE_TRI_NE: return v>0 ? 0 : x;
		case SHAPE_TRI_NW: return v>0 ? 0 : L-x;
		case SHAPE_TRI_SE: return v>0 ? L-x : 0;
		case SHAPE_TRI_SW: return v>0 ? x : 0;
		case SHAPE_LTRAP_FLOOR: return v>0 ? x/2 : L;
		case SHAPE_RTRAP_FLOOR: return v>0 ? L/2 - x/2 : L;
		case SHAPE_LSLOPE_FLOOR: return v>0 ? L/2 + x/2 : L;
		case SHAPE_RSLOPE_FLOOR: return v>0 ? L - x/2 : L;
		case SHAPE_LTRAP_CEIL: return v>0 ? 0 : L - x/2;
		case SHAPE_RTRAP_CEIL: return v>0 ? 0 : L/2 + x/2;
		case SHAPE_LSLOPE_CEIL: return v>0 ? 0 : L/2 - x/2;
		case SHAPE_RSLOPE_CEIL: return v>0 ? 0 : x/2;
		default: return 0;
	}
}

/* polarity functions
these help the top level collision tests by determining
which side of the rectangle needs to be traced. see x_trace. */
static int y_polarity(int y_0, int y_1, int shape){
	switch(shape){
		case SHAPE_FREE: return y_0;
		case SHAPE_SQUARE: return y_0;
		case SHAPE_TRI_NE: return y_0;
		case SHAPE_TRI_NW: return y_0;
		case SHAPE_TRI_SE: return y_1;
		case SHAPE_TRI_SW: return y_1;
		case SHAPE_LTRAP_FLOOR: return y_1;
		case SHAPE_RTRAP_FLOOR: return y_1;
		case SHAPE_LSLOPE_FLOOR: return y_1;
		case SHAPE_RSLOPE_FLOOR: return y_1;
		case SHAPE_LTRAP_CEIL: return y_0;
		case SHAPE_RTRAP_CEIL: return y_0;
		case SHAPE_LSLOPE_CEIL: return y_0;
		case SHAPE_RSLOPE_CEIL: return y_0;
		case SHAPE_HALF_FLOOR: return y_0;
		case SHAPE_HALF_CEIL: return y_0;
		default: return y_0;
	}
}

static int x_polarity(int x_0, int x_1, int shape){
	switch(shape){
		case SHAPE_FREE: return x_0;
		case SHAPE_SQUARE: return x_0;
		case SHAPE_TRI_NE: return x_1;
		case SHAPE_TRI_NW: return x_0;
		case SHAPE_TRI_SE: return x_1;
		case SHAPE_TRI_SW: return x_0;
		case SHAPE_LTRAP_FLOOR: return x_0;
		case SHAPE_RTRAP_FLOOR: return x_1;
		case SHAPE_LSLOPE_FLOOR: return x_0;
		case SHAPE_RSLOPE_FLOOR: return x_1;
		case SHAPE_LTRAP_CEIL: return x_0;
		case SHAPE_RTRAP_CEIL: return x_1;
		case SHAPE_LSLOPE_CEIL: return x_0;
		case SHAPE_RSLOPE_CEIL: return x_1;
		case SHAPE_HALF_FLOOR: return x_0;
		case SHAPE_HALF_CEIL: return x_0;
		default: return x_0;
	}
}

/*
return the corner tiles of the test rectangle in id.
if all corners are the same, return 0
if all corners are different, return 3
if the top two (and bottom two) are the same, return 2
if the left two (and right two) are the same, return 1
*/
static int calc_corners(int x, int y, int w, int h, int id[4]){
	int W = this_stage->w;

	int x1 = DDD(x)/16;
	int x2 = DDD(x+w)/16;
	int y1 = DDD(y)/16;
	int y2 = DDD(y+h)/16;

	id[0] = x1+y1*W;
	id[1] = x2+y1*W;
	id[2] = x1+y2*W;
	id[3] = x2+y2*W;

	/* single tile test */
	if(id[0] == id[1] && id[0] == id[2] &&
	   id[0] == id[2] && id[0] == id[3]) return 0;

	/* line of tile tests */
	if(id[0] == id[1]) return 2;
	if(id[1] == id[3]) return 3;

	/* at least 4 tile tests */
	return 1;
}


/* collision algorithm
strategy: if the rectangle adjusted by v overlaps something in
the stage, return 1 and set *xx or *yy to the collision point.

current holes:
for a large rectangle, it only tests the corners. it should
also do edge test or middle test on none corner (but not inner)
tiles.

the x collide and y collide are almost the same thing. it might
be possible to combine them into one function.

*/

int stage_xcollide(int x, int y, int w, int h, int v, int* xx){
	int corners[4];
	int type = calc_corners(x+v,y,w,h,corners);
//	int x_min = INT_MAX;
//	int x_max = INT_MIN;
	int L = UUU(16);
	int xbase = (x+v) / L * L;
	int x_ = (x+v) % L;
	int y_ = y % L;
	int yz;
	tile* t = NULL;

	if(v == 0) return 0;

	if(type==0){
		t = this_stage->tiles + corners[0];
		if(corner_overlap(x_,y_,1,1,t->shape) &&
		   corner_overlap(x_+w,y_,-1,1,t->shape) &&
		   corner_overlap(x_,y_+h,1,-1,t->shape) &&
		   corner_overlap(x_+w,y_+h,-1,-1,t->shape))
		{
			yz = y_polarity(y_,y_+h,t->shape);
			*xx = xbase+x_trace(yz,v,t->shape);
			return 1;
		}
		else{
			return 0;
		}
	}
	else if(type==2){
		t = this_stage->tiles + corners[0]; /* and 1 */
		if(corner_overlap(x_,y_,1,1,t->shape) &&
		   corner_overlap(x_+w,y_,-1,1,t->shape))
		{
			yz = y_polarity(y_,L,t->shape);
			*xx = xbase+x_trace(yz,v,t->shape);
			return 1;
		}

		y_ = (y+h) % L;
		t = this_stage->tiles + corners[2]; /* and 3 */
		if(corner_overlap(x_,y_,1,-1,t->shape) &&
		   corner_overlap(x_+w,y_,-1,-1,t->shape))
		{
			yz = y_polarity(0,y_,t->shape);
			*xx = xbase+x_trace(yz,v,t->shape);
			return 1;
		}

		/* also check middle edge */

		return 0;
	}
	else if(type==3){
		t = this_stage->tiles + corners[0]; /* and 2 */
		if(corner_overlap(x_,y_,1,1,t->shape) &&
		   corner_overlap(x_,y_+h,1,-1,t->shape))
		{
			yz = y_polarity(y_,y_+h,t->shape);
			*xx = xbase+x_trace(yz,v,t->shape);
			return 1;
		}

		xbase = (x+v+w) / L * L;
		x_ = (x+v+w) % L;
		t = this_stage->tiles + corners[1]; /* and 3 */
		if(corner_overlap(x_,y_,-1,1,t->shape) &&
		   corner_overlap(x_,y_+h,-1,-1,t->shape))
		{
			yz = y_polarity(y_,y_+h,t->shape);
			*xx = xbase+x_trace(yz,v,t->shape);
			return 1;
		}

		/* also check middle edge */

		return 0;
	}
	else if(type==1){
		t = this_stage->tiles + corners[0];
		if(corner_overlap(x_,y_,1,1,t->shape)){
			yz = y_polarity(y_,L,t->shape);
			*xx = xbase+x_trace(yz,v,t->shape);
			return 1;
		}

		y_ = (y+h)%L;
		t = this_stage->tiles + corners[2];
		if(corner_overlap(x_,y_,1,-1,t->shape)){
			yz = y_polarity(0,y_,t->shape);
			*xx = xbase+x_trace(yz,v,t->shape);
			return 1;
		}

		xbase = (x+v+w) / L * L;
		x_ = (x+v+w) % L;
		y_ = y % L;
		t = this_stage->tiles + corners[1];
		if(corner_overlap(x_,y_,-1,1,t->shape)){
			yz = y_polarity(y_,L,t->shape);
			*xx = xbase+x_trace(yz,v,t->shape);
			return 1;
		}

		y_ = (y+h)%L;
		t = this_stage->tiles + corners[3];
		if(corner_overlap(x_,y_,-1,-1,t->shape)){
			yz = y_polarity(0,y_,t->shape);
			*xx = xbase+x_trace(yz,v,t->shape);
			return 1;
		}
		/* check all edges and inside */

		return 0;
	}
	else{
		return 0;
	}
}

int stage_ycollide(int x, int y, int w, int h, int v, int* yy){
	int corners[4];
	int type = calc_corners(x,y+v,w,h,corners);
//	int x_min = INT_MAX;
//	int x_max = INT_MIN;
	int L = UUU(16);
	int ybase = (y+v) / L * L;
	int y_ = (y+v) % L;
	int x_ = x % L;
	int xz;
	tile* t = NULL;

	if(v == 0) return 0;

	if(type==0){
		t = this_stage->tiles + corners[0];
		if(corner_overlap(x_,y_,1,1,t->shape) &&
		   corner_overlap(x_+w,y_,-1,1,t->shape) &&
		   corner_overlap(x_,y_+h,1,-1,t->shape) &&
		   corner_overlap(x_+w,y_+h,-1,-1,t->shape))
		{
			xz = x_polarity(x_,x_+w,t->shape);
			*yy = ybase+y_trace(xz,v,t->shape);
			return 1;
		}
		else{
			return 0;
		}
	}
	else if(type==2){
		t = this_stage->tiles + corners[0]; /* and 1 */
		if(corner_overlap(x_,y_,1,1,t->shape) &&
		   corner_overlap(x_+w,y_,-1,1,t->shape))
		{
			xz = x_polarity(x_,x_+w,t->shape);
			*yy = ybase+y_trace(xz,v,t->shape);
			return 1;
		}

		ybase = (y+v+h) / L * L;
		y_ = (y+v+h) % L;
		t = this_stage->tiles + corners[2]; /* and 3 */
		if(corner_overlap(x_,y_,1,-1,t->shape) &&
		   corner_overlap(x_+w,y_,-1,-1,t->shape))
		{
			xz = x_polarity(x_,x_+w,t->shape);
			*yy = ybase+y_trace(xz,v,t->shape);
			return 1;
		}

		/* also check middle edge */

		return 0;
	}
	else if(type==3){
		t = this_stage->tiles + corners[0]; /* and 2 */
		if(corner_overlap(x_,y_,1,1,t->shape) &&
		   corner_overlap(x_,y_+h,1,-1,t->shape))
		{
			xz = x_polarity(x_,L,t->shape);
			*yy = ybase+y_trace(xz,v,t->shape);
			return 1;
		}

		x_ = (x+w) % L;
		t = this_stage->tiles + corners[1]; /* and 3 */
		if(corner_overlap(x_,y_,-1,1,t->shape) &&
		   corner_overlap(x_,y_+h,-1,-1,t->shape))
		{
			xz = x_polarity(0,x_,t->shape);
			*yy = ybase+y_trace(xz,v,t->shape);
			return 1;
		}

		/* also check middle edge */

		return 0;
	}
	else if(type==1){
		t = this_stage->tiles + corners[0];
		if(corner_overlap(x_,y_,1,1,t->shape)){
			xz = x_polarity(x_,L,t->shape);
			*yy = ybase+y_trace(xz,v,t->shape);
			return 1;
		}

		x_ = (x+w)%L;
		t = this_stage->tiles + corners[1];
		if(corner_overlap(x_,y_,-1,1,t->shape)){
			xz = x_polarity(0,x_,t->shape);
			*yy = ybase+y_trace(xz,v,t->shape);
			return 1;
		}

		ybase = (y+v+h) / L * L;
		y_ = (y+v+h) % L;
		x_ = x % L;
		t = this_stage->tiles + corners[2];
		if(corner_overlap(x_,y_,1,-1,t->shape)){
			xz = x_polarity(x_,L,t->shape);
			*yy = ybase+y_trace(xz,v,t->shape);
			return 1;
		}

		x_ = (x+w)%L;
		t = this_stage->tiles + corners[3];
		if(corner_overlap(x_,y_,-1,-1,t->shape)){
			xz = x_polarity(0,x_,t->shape);
			*yy = ybase+y_trace(xz,v,t->shape);
			return 1;
		}

		/* check all edges and inside */

		return 0;
	}
	else{
		return 0;
	}
}




/*** end collision stuff ***/




static void initialize_tiles(int n, tile* tiles){
	int i;
	for(i=0; i<n; i++){
		tiles[i].shape = SHAPE_FREE;
		tiles[i].fg = 0;
		tiles[i].bg = 0;
	}
}


static int load_stage(char* gfxpath, char* stagepath){
/*
[stage file]
width height
bgimage
dectiles
bgtiles
fgtiles
x y fg bg shape
x y fg bg shape
...
*/

	reader* f = loader_open(stagepath);
	char* filename = path_ending(stagepath);
	char buf[256];
	char gfx[256];
	tile* tptr;
	stage* s = malloc(sizeof(stage));
	int w = 20;
	int h = 15;
	int x, y, ox, oy, fg, bg, shape;

	loader_scanline(f, "%d %d %d %d", &w, &h, &ox, &oy);

	loader_scanline(f, "%s", buf);
	strcpy(gfx, gfxpath);
	strcat(gfx, buf);
	s->bgimage = load_bitmap(gfx);

	loader_scanline(f, "%s", buf);
	strcpy(gfx, gfxpath);
	strcat(gfx, buf);
	s->bgtiles = load_bitmap(gfx);

	loader_scanline(f, "%s", buf);
	strcpy(gfx, gfxpath);
	strcat(gfx, buf);
	s->fgtiles = load_bitmap(gfx);

	s->tiles = malloc(w*h*sizeof(tile));
	initialize_tiles(w*h, s->tiles);
	s->w = w;
	s->h = h;
	s->ox = ox;
	s->oy = oy;
	strncpy(s->id, filename, 256);
	s->id[255] = '\0';

	while(!loader_feof(f)){
		loader_scanline(f, "%d %d %d %d %c", &x, &y, &fg, &bg, &shape);
		tptr = s->tiles + (x+ox) + (s->w * (y+oy));
		tptr->fg = fg;
		tptr->bg = bg;
		tptr->shape = shape;
	}

	s->next = stages;
	stages = s;

	loader_close(f);
	return 0;
}

/***/


int load_zone(string name){
	/*
loading a zone from a file
a zone consists of one or more stages
each stage has its own tileset graphics and background
it also has a large array of tiles

the stage files are in zones/zonename/
the name of the file will be used as the id
	*/

	list* dirs;
	list* ptr;
	char stagesdir[256] = "";
	char gfxpath[256] = "";
	char* stagepath;

	strcat(stagesdir, "zones/");
	strcat(stagesdir, name);
	strcat(stagesdir, "/stages/");

	strcat(gfxpath, "zones/");
	strcat(gfxpath, name);
	strcat(gfxpath, "/gfx/");

	strncpy(zone_name, name, 256);
	zone_name[255] = '\0';

	dirs = loader_readdir((string)stagesdir);
	if(dirs == NULL){
		printf("ERROR cant read dirs\n");
		return -1;
	}

	ptr = dirs->next;
	while(ptr){
		stagepath = ptr->item;
		if(load_stage(gfxpath, stagepath) < 0){
			printf("ERROR cant load stage\n");
			loader_freedirlist(dirs);
			return -1;
		}
		ptr = ptr->next;
	}

	loader_freedirlist(dirs);
	return 0;
}

void unload_zone(){
	stage* ptr = stages;
	stage* prev;
	while(ptr){
		free(ptr->tiles);
		prev = ptr;
		ptr = ptr->next;
		free(prev);
	}

	strcpy(zone_name, "NO ZONE");
}

void switch_stage(string id){
	stage* ptr = stages;
	while(ptr){
		if(strcmp(id, ptr->id) == 0){
			this_stage = ptr;
			return;
		}
		ptr = ptr->next;
	}

	printf("ERROR stage not found\n");
}


static void draw_tiles(int gfx, char layer, int cx, int cy){
	tile* tbase = this_stage->tiles;
	tile* t;
	int tw = this_stage->w;
	int th = this_stage->h;
	int id;
	int gx, gy;

	int i;
	int j;
	int io = cx/16;
	int jo = cy/16;
	int i_ = io + 20;
	int j_ = jo + 15;

	int extra_x = 0;
	int extra_y = 0;
	int W, H;
	screen_dimensions(&W, &H);
	if(W>320){
		extra_x = (W - 320)/16/2 + 2;
	}
	if(H>240){
		extra_y = (H - 240)/16/2 + 2;
	}

	io -= extra_x;
	jo -= extra_y;
	i_ += extra_x;
	j_ += extra_y+1;

	if(io > tw-1) return;
	if(jo > th-1) return;

	if(io < 0) io = 0;
	if(jo < 0) jo = 0;
	if(i_ > tw) i_ = tw;
	if(j_ > th) j_ = th;

	for(j=jo; j<j_; j++){
		for(i=io; i<i_; i++){
			t = tbase + i + tw*j;
			if(layer=='b') id = t->bg;
			else if(layer=='f') id = t->fg;
			else id = 0;
			gx = 16*(id%16);
			gy = 16*(id/16);
			draw_gfx(gfx, i*16-cx, j*16-cy, gx, gy, 16, 16);
		}
	}
}

/*
cx, cy is the camera position
x, y i think is the offset from 0,0 you should shift
w, h i think is the width and height of the screen
*/
void stage_draw_bg(int cx, int cy){
	int i;
	int bw, bh, screen_w, screen_h;
	int bgimage;
	int bgshift;
	int parallax = 2;

	if(this_stage == NULL) return;

	/* draw background vertically centered
	tile horizontally at fraction of camera x */
	bgimage = this_stage->bgimage;
	gfx_dimensions(bgimage, &bw, &bh);
	screen_dimensions(&screen_w, &screen_h);
	bgshift = cx/parallax/bw;
	for(i=-1; i<(screen_w/bw)+2; i++){
		draw_gfx_raw(
			bgimage,
			(i+bgshift)*bw-cx/parallax,
			(screen_h-bh)/2,
			0, 0, bw, bh
		);
	}

	draw_tiles(this_stage->bgtiles, 'b', cx, cy);
}

void stage_draw_fg(int cx, int cy){
	draw_tiles(this_stage->bgtiles, 'f', cx, cy);
}




void stage_init(){
	/* does nothing */
}





/*** debug ***/

void stage_debug(){
	stage* ptr = stages;
	int i, j;
	char c;

	printf("stage debug:\n\n");
	printf("zone: %s\n\n", zone_name);

	while(ptr){
		printf("stage: %s\n", ptr->id);
		printf("size: %dx%d\n", ptr->w, ptr->h);
		printf("origin: (%d, %d)\n", ptr->ox, ptr->oy);
		printf("bgimage: %d\n", ptr->bgimage);
		printf("bgtiles: %d\n", ptr->bgtiles);
		printf("fgtiles: %d\n", ptr->fgtiles);
		printf("tiles:\n");

		for(j=0; j<ptr->h; j++){
			for(i=0; i<ptr->w; i++){
				c = (ptr->tiles + i + ptr->w*j)->fg;
				/*
				if(isprint(c)) printf("%d", c);
				else printf("?");*/
				printf("[%02x]", c);
			}
			printf("\n");
		}


		printf("\n");
		ptr = ptr->next;
	}
}
