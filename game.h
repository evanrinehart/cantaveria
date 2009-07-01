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

struct game {
  struct handler handler;
  void (*update)();
  int end;
};


struct tile {
  unsigned char type;
  unsigned char id;
};

struct screen {
  struct tile tiles[20][15];
  int flags;
};

typedef struct {
  struct screen* screens;
  int tileset;
  /*background*/
  int x,y,w,h;
} zone;


extern struct game game;
extern zone* zones[32];

enum {
SPRITE_ONE,
SPRITE_TWO,
SPRITE_THREE,
SPRITE_FOUR
};


void load_game();

