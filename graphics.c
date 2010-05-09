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
/* graphics */

/* these graphics routines wrap video.c low level stuff */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

#include <list.h>
#include <util.h>
#include <kernel.h>
#include <video.h>
#include <graphics.h>
#include <loader.h>
#include <camera.h>


/* graphics data */
sprite* sprites[MAX_SPRITES];
int sprite_count = 0;

animation* animations[MAX_ANIMATIONS];
int anim_count = 0;

int minifont_gfx = 0;



int stage_enabled = 0;



void graphics_init(){
	int i;
	minifont_gfx = load_gfx("smallfont.tga");
	for(i=0; i<MAX_ANIMATIONS; i++){
		animations[i] = NULL;
	}
}


/* drawing */

void draw_sprite(sprite* spr){
	int x = spr->x - camera_x();
	int y = spr->y - camera_y();
	int W = spr->w;
	int H = spr->h;
	int X = spr->frame.x;
	int Y = spr->frame.y;
	int g = spr->gfxid;
	draw_gfx(g, x, y, X, Y, W, H);
}

/*
   void draw_screen(zone* z, int si, int sj){
   struct screen* scr = z->screens+si+z->w*sj;
   int G = z->tileset;
   int x = si*20*16 - camera.x;
   int y = sj*15*16 - camera.y;

   for(int j=0; j < 15; j++){
   for(int i=0; i < 20; i++){
   if(x > 320 || y > 240 || x < -16 || y < -16) continue;
   int id = scr->tiles[i][j].id;
   if(id != 0){
   int X = id&7;
   int Y = id>>3;
   draw_gfx(G, x, y, X, Y, 16, 16);
   }
   else{
//draw background
}
x += 16;
}
x -= 320;
y += 16;
}
}*/









void draw_small_text(char* str, int x, int y){
	char* c;
	for(c=str; *c; c++){
		int X = *c & 0x0f;
		int Y = *c >> 4;
		draw_gfx_raw(minifont_gfx, x, y, X*4, Y*9, 3, 8);
		x+=4;
	}
}



void printf_small(int x, int y, char* format, ...){
	char str[128];
	va_list ap;
	va_start(ap, format);
	vsnprintf(str, 128, format, ap);
	va_end(ap);
	str[127]='\0';
	draw_small_text(str, x, y);
}




void draw_sprites(){
	int i;
	for(i=0; i<sprite_count; i++){
		draw_sprite(sprites[i]);
	}
}

void draw_final(){
	//fps_draw();
	update_video();
	clear_video();
}






void animate_sprite(int i){
	sprite* spr = sprites[i];

//	spr->frame_counter += dt;
	animation* ani = animations[spr->anim];


	while(spr->frame_counter > ani->frame_lens[spr->current_frame]){
		spr->frame_counter -= ani->frame_lens[spr->current_frame];
		spr->current_frame++;
		if(spr->current_frame == ani->frame_count){
			spr->current_frame = 0;
		}
		spr->frame = ani->frames[spr->current_frame];
	}

	//if(spr->update) spr->update(spr, spr->userdata);
}

void animate_sprites(){
	int i;
	for(i=0; i<sprite_count; i++){
		animate_sprite(i);
	}
}




int load_sprite(char* filename, int id){
	int i;

	//printf("loading %s\n",filename);

	char path[1024] = "sprites/";
	strncat(path, filename, 1023 - strlen(filename));

	reader* rd = loader_open(path);
	if(!rd){
		return -1;
	}

	animation* ani = xmalloc(sizeof(animation));

	char str[256];
	int w, h;
	int frame_count;
	int loop_mode;

	loader_scanline(rd,"%256s",str);
	loader_scanline(rd,"%d %d %d %d",&w,&h,&loop_mode,&frame_count);

	ani->frame_lens = xmalloc(frame_count*sizeof(short));
	ani->frames = xmalloc(frame_count*sizeof(struct frame));
	ani->frame_count = frame_count;

	int g = load_gfx(str);
	if(g < 0)
		return -1;

	ani->gfxid = g;

	//int W = gfx[g].w;
	//int H = gfx[g].h;
	int W = gfx_width(g);
	int H = gfx_height(g);
	ani->w = w;
	ani->h = h;

	for(i=0; i < frame_count; i++){
		int l, x, y;
		loader_scanline(rd, "%d %d %d", &l, &x, &y);
		ani->frame_lens[i] = l;
		ani->frames[i].x = x;
		ani->frames[i].y = y;
		ani->frames[i].x0 = ((double)x)/W;
		ani->frames[i].y0 = ((double)y)/H;
		ani->frames[i].x1 = ((double)(x+w))/W;
		ani->frames[i].y1 = ((double)(y+h))/W;
	}

	loader_close(rd);
	animations[id] = ani;
	return 0;
}




/********************/
/* graphics control */
/********************/

sprite* enable_sprite(int sprnum){
	if(!animations[sprnum]){
		fatal_error("enable_sprite: you just tried to enable sprite with type %d, which does not exist\n",sprnum);
	}
	if(sprite_count == MAX_SPRITES){
		/* need a priority based way to create important sprites if full */
		return NULL;
	}
	sprite* spr = xmalloc(sizeof(sprite));
	animation* ani = animations[sprnum];

	spr->number = sprite_count;
	spr->frame_counter = 0;
	spr->current_frame = 0;
	spr->frame = ani->frames[0];
	spr->gfxid = ani->gfxid;
	spr->anim = sprnum;
	spr->x = 0;
	spr->y = 0;
	spr->w = ani->w;
	spr->h = ani->h;
	//spr->vx = 0;
	//spr->vy = 0;
	//spr->update = NULL;
	//spr->userdata = NULL;

	sprites[sprite_count++] = spr;
	return spr;
}

void disable_sprite(sprite* spr){
	sprite* tmp = sprites[spr->number];
	sprites[spr->number] = sprites[sprite_count--];
	free(tmp);
}

sprite* copy_sprite(sprite* spr){
	if(sprite_count == MAX_SPRITES){
		/* need way to make important sprites when full */
		return NULL;
	}
	sprite* copy = xmalloc(sizeof(sprite));
	*copy = *spr;
	sprites[sprite_count++] = copy;
	return copy;
}



void enable_stage(int yn){
	stage_enabled = yn;
}



int load_bitmap(char* filename){
	int g = load_gfx(filename);
	return g;
}

void draw_bitmap(int id, int x, int y){
	int W = gfx_width(id);
	int H = gfx_height(id);
	draw_gfx(id, x, y, 0, 0, W, H);
}
