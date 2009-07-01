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

#include "util.h"

enum {
ESCAPE_KEY,
PAUSE_KEY,

LEFT_KEY,
RIGHT_KEY,
UP_KEY,
DOWN_KEY,

FIRE_KEY,
JUMP_KEY,
INVENTORY_KEY,
SPECIAL_KEY,

L_KEY,
R_KEY,
START_KEY,
SELECT_KEY
};

enum{
FIRE_BUTTON,
JUMP_BUTTON,
INVENTORY_BUTTON,
SPECIAL_BUTTON,
L_BUTTON,
R_BUTTON,
START_BUTTON,
SELECT_BUTTON
};

enum{
KEYUP,
KEYDOWN,
JOYMOVEX,
JOYMOVEY,
JOYPRESS,
JOYRELEASE,
FOOBAR
};

#define VERSION_MAJOR 0
#define VERSION_MINOR 0

#define JOY_MAX 32767
#define JOY_MIN -32768

#define MAX_PLAYERS 6

#define dt 10
#define DELAY_AMOUNT 1

#define SCREEN_W 320
#define SCREEN_H 240

#define MAX_GFX 256
#define MAX_SPRITES 256
#define MAX_ANIMATIONS 256

#define COLOR_KEY 0x000000

#define SAMPLE_RATE 44100
#define BUFFER_SIZE 1024

#define RANDOM_SEED 57

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
  int x, y, w, h, vx, vy;
  void (*update)(sprite*, void* ud);
  void* userdata;
};

typedef struct {
  int w, h;
  int loop_mode;/*0 stopped, 1 once, 2 repeat, 3 pingpong, 4 evaporate*/
  int gfxid;
  int frame_count;
  short* frame_lens;
  struct frame* frames;
} animation;



void backend_init(int argc, char* argv[]);
void backend_quit();

void input(); /* pump event system */
void draw();  /* draw all active graphics */

void animate_sprites();

int since(); /* ms since last time since() was called */
void delay(int ms); /* wait ms ms */

/* input */
int keynum(int name); /* get key number for key name */
int butnum(int joy, int name); /* get button number for button name */
void control(int type, int par1, int par2); /* automatic control */

/* gfx control */
int load_gfx(char* filename);
int load_sprite(char* filename, int id);
int load_font(char* filename);
sprite* enable_sprite(int sprnum);
void disable_sprite(sprite* spr);
sprite* copy_sprite(sprite* spr);
void point_camera(int x, int y);


/* sound control */
int load_sound(char* filename);
void play_sound(int id);
int load_music(char* filename);
int play_music(int id);

/* stage */
void load_stage(char* filename);
void unload_stage();


/* text */
sprite* small_text(char* str);
void draw_small_text(char* str, int x, int y);
void printf_small(int x, int y, char* format, ...);

void set_message(char* str);
void advance_message();
void clear_message();
void complete_message();

void reposition_message(int x, int y);
void resize_message(int w, int h);
