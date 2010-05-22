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
* calculate if a moving rectangle will collide with world
*/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	//int th = this_stage->h;
	int id;
	int gx, gy;

	int i;
	int j;
	int io = cx/16;
	int jo = cy/16;

	for(j=jo; j<jo+20; j++){
		for(i=io; i<io+15; i++){
			t = tbase + i + tw*j;
			if(layer=='b') id = t->bg;
			else if(layer=='f') id = t->fg;
			else id = 0;
			gx = 16*(id%16);
			gy = 16*(id/16);
			draw_gfx(gfx, 0, 0, gx, gx, 16, 16);
		}
	}
}

/*
cx, cy is the camera position
x, y i think is the offset from 0,0 you should shift
w, h i think is the width and height of the screen
*/
void stage_draw_bg(int cx, int cy, int x, int y, int w, int h){
	int i;
	int bw, bh, screen_w, screen_h;
	int bgimage;
	int bgshift;
	int parallax = 1;

	if(this_stage == NULL) return;

	/* draw background vertically centered
	tile horizontally at fraction of camera x */
	bgimage = this_stage->bgimage;
	gfx_dimensions(bgimage, &bw, &bh);
	screen_dimensions(&screen_w, &screen_h);
	bgshift = cx/bw;
	for(i=-1; i<(screen_w/bw)+2; i++){
		draw_gfx_raw(
			bgimage,
			(i+bgshift)*bw-cx/parallax,
			(screen_h-bh)/2,
			0, 0, bw, bh
		);
	}

	draw_tiles(this_stage->bgtiles, '0', cx, cy);
}

void stage_draw_fg(int cx, int cy, int x, int y, int w, int h){
	//draw 2 if not blank
}



int stage_xcollide(int x, int y, int w, int h, int v, int* xx){
	return 0;
}

int stage_ycollide(int x, int y, int w, int h, int v, int* yy){
	return 0;
}

void stage_init(){
	/* does nothing */
}





/*** debug ***/

void stage_debug(){
	stage* ptr = stages;
	int i, j;
	char c;
/*
struct stage {
	char id[32];
	int w, h;
	tile* tiles;
	int bgimage;
	int bgtiles;
	int fgtiles;
	stage* next;
};*/
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
				c = (ptr->tiles + i + ptr->w*j)->bg;
				printf("%d", c);
			}
			printf("\n");
		}


		printf("\n");
		ptr = ptr->next;
	}
}
