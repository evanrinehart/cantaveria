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

#define JOY_MAX 32767
#define JOY_MIN -32768

#define MAX_PLAYERS 6

#define dt 20
#define DELAY_AMOUNT 20

typedef struct sprite sprite;
struct sprite {
  int number;
  int frame_c;
  int frame_counter;
  int current_frame;
  int gfx;
  int frame_len[16];
  int loop_mode;/*0 stopped, 1 once, 2 repeat, 3 pingpong, 4 evaporate*/
  int x, y, w, h, vx, vy;
  void (*update)(sprite*, void* ud);
  void* userdata;
};


void backend_init(int argc, char* argv[]);
void backend_quit();

void input(); /* pump event system */
void draw();  /* draw all active graphics */


int since(); /* ms since last time since() was called */
void delay(int ms); /* wait ms ms */

int keynum(int name); /* get key number for key name */
int butnum(int joy, int name); /* get button number for button name */
void control(int type, int par1, int par2); /* automatic control */

int load_gfx(char* filename);
int load_sprite(char* filename, int sprnum);
sprite* enable_sprite(int sprnum);
void disable_sprite(sprite* spr);
sprite* copy_sprite(sprite* spr);

void load_map(char* filename);
void unload_map();

void point_camera(int x, int y);

void animate_sprites();