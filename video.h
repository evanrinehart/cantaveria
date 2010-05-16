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



#define SCREEN_W 320
#define SCREEN_H 240

#define MAX_GFX 256


#define COLOR_KEY 0x000000




void video_init(int argc, char* argv[]);
void video_quit();

int since(); /* ms since last time since() was called */
void delay(int ms); /* wait ms ms */

void update_video();
void clear_video();

/* gfx control */
int load_gfx(char* filename);
void draw_gfx(int gfxid, int x, int y, int X, int Y, int W, int H);
void draw_gfx_raw(int gfxid, int x, int y, int X, int Y, int W, int H);
int gfx_width(int gfxid);
int gfx_height(int gfxid);

void draw_black_rect(int x, int y, int w, int h);

void fps_update();
void fps_draw();
int get_fps();

void load_panic_gfx();
void map_pixel(int mx, int my, int *x, int *y);
