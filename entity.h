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

typedef struct entity Y;
typedef struct {int x,y,w,h;} rect;
typedef struct {int a,b,c,d,e,f;} memory;

struct entity {
	int x; int y; /* spatial */
	int z; /* z order */
	int flbits; /* flag bits */
	int gfx; /* image */
	memory r; /* general registers */
	rect box; /* used to detect collision */
	int (*update)(Y*);
	int (*stage)(Y*,struct stghit);
	int (*overlap)(Y*,Y*);
	int (*diverge)(Y*,Y*);
};

void draw_entities(void);
void setup_test_entities(void);
void entity_master_simulate(void);

void player_press(int player, enum input_button button);
void player_release(int player, enum input_button button);

Y* mk_dummy_ent(int x, int y);
int mini_rand(int g);




