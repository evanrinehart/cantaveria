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
#define MAX_SPRITES 256
#define MAX_ANIMATIONS 256

struct frame{
  int x, y;
  double x0, y0, x1, y1;
};

typedef struct sprite sprite;
struct sprite {
  int number;
  int frame_counter;
  int current_frame;
  struct frame frame;
  int gfxid;
  int anim;
  int x, y, w, h;
};

typedef struct {
  int w, h;
  int loop_mode;/*0 stopped, 1 once, 2 repeat, 3 pingpong, 4 evaporate*/
  int gfxid;
  int frame_count;
  short* frame_lens;
  struct frame* frames;
} animation;


void draw_final();
void draw_sprites();


int load_sprite(string filename, int id);

int load_bitmap(string filename);
void draw_bitmap(int id, int x, int y);

sprite* enable_sprite(int sprnum);
void disable_sprite(sprite* spr);
sprite* copy_sprite(sprite* spr);

void draw_small_text(string str, int x, int y);
void printf_small(int x, int y, const char* format, ...);


void set_message(char* str);
void advance_message();
void clear_message();
void complete_message();

void reposition_message(int x, int y);
void resize_message(int w, int h);

void animate_sprites();


void enable_stage(int en);



