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

struct handler {
  void (*keydown)(int key);
  void (*keyup)(int key);
  void (*joymovex)(int joy, int x);
  void (*joymovey)(int joy, int y);
  void (*joypress)(int joy, int button);
  void (*joyrelease)(int joy, int button);
};


struct screen {
  unsigned char tiles[20][15];
  int flags;
  int exits[4];
};

typedef struct {
  char* name;
  struct screen** screens;
  int tileset_gfx;
  char tileset_shapes[256];
  /*background*/
  int x,y,w,h;
} zone;

#define ZONE_LOOKUP(Z,I,J) (I >= Z->w || J >= Z->h || I < 0 || J < 0 ? NULL : *(Z->screens + I + J*Z->w))

struct game {
  struct handler handler;
  void (*update)();
  void (*draw)();
  int end;

  rng_state rng;

  zone* zones[32];
  int zone_count;

  /*these track the location of the player*/
  int player_x;
  int player_y;
  int current_zone;
  int si, sj;
};

extern struct game game;

enum {
SPRITE_ONE,
SPRITE_TWO,
SPRITE_THREE,
SPRITE_FOUR,
SPR_BOX
};


void load_game();

void load_zone(char* filename);
