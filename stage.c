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

#include <stage.h>



enum tile_shape {
SHAPE_FREE, /* 'air' */
SHAPE_SQUARE,
SHAPE_NULL, /*does not exist*/

SHAPE_TRI_NE,
SHAPE_TRI_NW,
SHAPE_TRI_SE,
SHAPE_TRI_SW,

SHAPE_LTRAP_FLOOR, /* trapezoid */
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
	unsigned char bg;
	unsigned char fg;
	unsigned char shape;
} tile;

typedef struct stage stage;
struct stage {
	char id[32];
	int w, h;
	tile* tiles;
	int bgimage;
	int dectiles;
	int bgtiles;
	int fgtiles;
	stage* next;
};

char zone_name[32] = "NO ZONE";
stage* stages = NULL;
stage* this_stage = NULL;

/***/


/*
STAGE MODULE

DEADLINES AND MILESTONES

load zones / stages from new format - May 15
  this may be done.
  needs testing (print out the stage after loading)
display loaded stages - May 22
collision algorithm - May 29

*/



static int load_stage(char* path){
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

	reader* f = loader_open(path);
	char* filename = path_ending(path);
	char buf[256];
	tile* tptr;
	stage* s = malloc(sizeof(stage));
	int w = 20;
	int h = 15;
	int x, y, fg, bg, shape;

	loader_scanline(f, "%d %d", &w, &h);
	loader_scanline(f, "%s", buf);
	s->bgimage = load_bitmap(buf);
	loader_scanline(f, "%s", buf);
	s->dectiles = load_bitmap(buf);
	loader_scanline(f, "%s", buf);
	s->bgtiles = load_bitmap(buf);
	loader_scanline(f, "%s", buf);
	s->fgtiles = load_bitmap(buf);

	s->tiles = malloc(w*h*sizeof(tile));
	s->w = w;
	s->h = h;
	strcpy(s->id, filename);

	while(!loader_feof(f)){
		loader_scanline(f, "%d %d %d %d %d", &x, &y, &fg, &bg, &shape);
		tptr = s->tiles + x + (s->w * y);
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


int load_zone(char* name){
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
	char path[256] = "";
	char* filename;

	/* "zones/" ++ filename */
	strncat(path, "zones/", 256); /* check for correctness */
	strncat(path, name, 256 - strlen("zones/"));
	strncpy(zone_name, name, 32);
	zone_name[31] = '\0';

	dirs = loader_readdir(path);
	if(dirs == NULL){
		printf("ERROR cant read dirs\n");
		return -1;
	}

	ptr = dirs->next;
	while(ptr){
		filename = ptr->item;
		if(load_stage(filename) < 0){
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
}

void switch_stage(char* id){
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

void draw_stage_fg(int cx, int cy, int x, int y, int w, int h){
	//draw background
		/* draw background on tiles where at least
		the fg or bg tile is partial*/
	//draw bg tiles
		/* draw bg tile where fg is partial */
}

void draw_stage_bg(int cx, int cy, int x, int y, int w, int h){
	//draw water
		/* calculate water surfaces, draw them  */
	//fg tiles
		/* draw all fg visible fg tiles */
	//draw decorations
		/* draw all decorations */
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


