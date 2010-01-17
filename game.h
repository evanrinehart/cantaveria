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


#define PIXUP 1024

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

	int bg_gfx;
} zone;

#define ZONE_LOOKUP(Z,I,J) (I >= Z->w || J >= Z->h || I < 0 || J < 0 ? NULL : *(Z->screens + (I) + (J)*Z->w))

struct box{int x,y,w,h;};

typedef struct {
	struct box box; /*absolute coords*/
	int x, y; /*absolute coords*/
	int vx, vy; /* pixels per ms / 256 */
	int xoff, yoff; /* pixel coords*/
	int bxoff, byoff; /* absolute coords */
	enum {LEFT, RIGHT} facing;
	sprite* spr; /*pixel coords*/
	zone* z;
	int si, sj;
	int flags;
} mobile;

typedef struct {
	struct box box;
	int type;
} bullet;

typedef struct {
	struct box box;
	//sprite* spr;
	int params[6];
	int t;
} moveplat;


struct player_motion {
	int state;
	int timer;

};


struct game {
	void (*update)();
	void (*draw)();

	zone* zones[32];
	int zone_count;

	/*these track the location of the player*/
	int player_x;
	int player_y;
	zone* current_zone;
	int si, sj;


	/* entities */

};

extern struct game game;

enum {
	SPRITE_ONE,
	SPRITE_TWO,
	SPRITE_THREE,
	SPRITE_FOUR,
	SPR_BOX
};


void load_zone(char* filename);


int stage_collision(mobile* m, zone* z, int si, int sj);
int box_collision(struct box* B1, struct box* B2);

void update_mobile_motion(mobile* m);


void game_setup();
