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
* calculate if and where a moving rectangle will collide with world
*/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#include <util.h>
#include <list.h>
#include <loader.h>
#include <graphics.h>
#include <video.h>

#include <stage.h>



enum tile_shape {
SHAPE_FREE = '0', /* 'air' */
SHAPE_SQUARE = 'M',

SHAPE_TRI_NE = 'a',
SHAPE_TRI_NW = 'b',
SHAPE_TRI_SE = 'c',
SHAPE_TRI_SW = 'd',

SHAPE_LTRAP_FLOOR = '1', /* trapezoid */
SHAPE_RTRAP_FLOOR = '2',
SHAPE_LSLOPE_FLOOR = '3',
SHAPE_RSLOPE_FLOOR = '4',
SHAPE_LTRAP_CEIL = '5',
SHAPE_RTRAP_CEIL = '6',
SHAPE_LSLOPE_CEIL = '7',
SHAPE_RSLOPE_CEIL = '8',

SHAPE_HALF_FLOOR = 'm',
SHAPE_HALF_CEIL = 'w'
};

typedef struct stghit Q;

typedef struct {
	unsigned char bg;
	unsigned char fg;
	unsigned char shape;
	unsigned char config; /* maintest | sideconfig */
} tile;

typedef struct stage stage;
struct stage {
	char id[256];
	int w, h;
	tile* tiles;
	int bgimage;
	int bgtiles;
	int fgtiles;
	int ox, oy;
	stage* next;
};

typedef Q (*test)(int x, int y, int vx, int vy);

static test maintests[16];
static test sidetests[4];

char zone_name[256] = "NO ZONE";
stage* stages = NULL;
stage* this_stage = NULL;

#define SIN26(X) ((X) * 458 / 1024)
#define COS26(X) ((X) * 916 / 1024)
#define SIN45(X) ((X) * 724 / 1024)
#define COS45(X) ((X) * 724 / 1024)
#define UUU(X) ((X) * 1024)
#define DDD(X) ((X) / 1024)

static const int L = UUU(16);
static const struct stghit nohit = {0,0,0,0,0,0,0};


static unsigned char pack_config(int maintest, int sideconfig){
	return (maintest << 4) | sideconfig;
}

static void unpack_config(unsigned char config, int* mtout, int* scout){
	*scout = config & 0x0f;
	*mtout = (config & 0xf0) >> 4;
}

static void get_sidetests(int sideconfig, test tests[3]){
	int i = 0;
	int sc = sideconfig;
	tests[0] = NULL;
	tests[1] = NULL;
	tests[2] = NULL;

	if(sc==0xf) {
		printf("stage: problem with tile config\n");
		return;
	}

	if(sc&1) tests[i++]=sidetests[0];
	if(sc&2) tests[i++]=sidetests[1];
	if(sc&4) tests[i++]=sidetests[2];
	if(sc&8) tests[i++]=sidetests[3];
}



static int min4(int a, int b, int c, int d){
	int m = INT_MAX;
	if(a < m) m = a;
	if(b < m) m = b;
	if(c < m) m = c;
	if(d < m) m = d;
	return m;
}

static int min3(int a, int b, int c){
	int m = INT_MAX;
	if(a < m) m = a;
	if(b < m) m = b;
	if(c < m) m = c;
	return m;
}

static int max4(int a, int b, int c, int d){
	int m = INT_MIN;
	if(a > m) m = a;
	if(b > m) m = b;
	if(c > m) m = c;
	if(d > m) m = d;
	return m;
}

static struct stghit mkhit(int x, int y, int vx, int vy, int s, int z){
	struct stghit hit;
	hit.yes = 1;
	hit.x = x;
	hit.y = y;
	hit.vx = vx;
	hit.vy = vy;
	hit.surface = s;
	hit.depth = z;
	return hit;
}


static Q st_left(int x, int y, int vx, int vy){
	return mkhit(-1,y,-vx,vy,1,x);
}
static Q st_right(int x, int y, int vx, int vy){
	return mkhit(L,y,-vx,vy,1,L-x);
}
static Q st_top(int x, int y, int vx, int vy){
	return mkhit(x,-1,vx,-vy,0,y);
}
static Q st_bottom(int x, int y, int vx, int vy){
	return mkhit(x,L,vx,-vy,2,L-y);
}




static Q mt_square(int x, int y, int vx, int vy){
	int z1,z2,z3,z4,z;
	int xx, yy, vvx, vvy;
	z1 = x;
	z2 = y;
	z3 = L - x;
	z4 = L - y;
	z = min4(z1,z2,z3,z4);
	if(z == z1) return mkhit(-1,y,-vx,vy,1,z);
	if(z == z2) return mkhit(x,-1,vx,-vy,0,z);
	if(z == z3) return mkhit(L,y,-vx,vy,1,z);
	if(z == z4) return mkhit(x,L,vx,-vy,2,z);
	return nohit;
}

static Q mt_half_floor(int x, int y, int vx, int vy){
	if(y < L/2) return nohit;
	else return mkhit(x,L/2,vx,-vy,0,y-L/2);
}

static Q mt_half_ceil(int x, int y, int vx, int vy){
	if(y > L/2) return nohit;
	else return mkhit(x,L/2,vx,-vy,0,L/2-y);
}


static Q mt_ltrap_floor(int x, int y, int vx, int vy){
	int dot, z;
	if(y < x/2) return nohit;
	z = COS26(y - x/2);
	dot = SIN26(vx) - COS26(vy);
	return mkhit(
		x + SIN26(z),
		y - COS26(z),
		vx - SIN26(2 * dot),
		vy - -COS26(2 * dot),
		0,
		z
	);
}
static Q mt_rtrap_floor(int x, int y, int vx, int vy){
	return nohit;
}

static Q mt_ltrap_ceil(int x, int y, int vx, int vy){
	return nohit;
}

static Q mt_rtrap_ceil(int x, int y, int vx, int vy){
	return nohit;
}

static Q mt_lslope_floor(int x, int y, int vx, int vy){
	int dot, z;
	if(y < x/2 + L/2) return nohit;
	z = COS26(y - x/2 - L/2);
	dot = SIN26(vx) - COS26(vy);
	return mkhit(
		x + SIN26(z),
		y - COS26(z),
		vx - SIN26(2 * dot),
		vy - -COS26(2 * dot),
		0,
		z
	);
}
static Q mt_rslope_floor(int x, int y, int vx, int vy){
	return nohit;
}

static Q mt_lslope_ceil(int x, int y, int vx, int vy){
	return nohit;
}

static Q mt_rslope_ceil(int x, int y, int vx, int vy){
	return nohit;
}


static Q mt_triangle_ne(int x, int y, int vx, int vy){
	return nohit;
}

static Q mt_triangle_nw(int x, int y, int vx, int vy){
	return nohit;
}

static Q mt_triangle_se(int x, int y, int vx, int vy){
	return nohit;
}

static Q mt_triangle_sw(int x, int y, int vx, int vy){
	int z,dot;
	if(y < x) return nohit;
	z = COS45(y - x);
	dot = SIN45(vx) - COS45(vy);
	return mkhit(
		x + COS45(z),
		y - SIN45(z),
		vx - COS45(2 * dot),
		vy - -SIN45(2 * dot),
		0,
		z
	);
}

static Q nulltest(int x, int y, int vx, int vy){
	return nohit;
}


static Q general_test(
	int x, int y, int vx, int vy,
	test maintest, test side1, test side2, test side3
) {
	int sides = 3;
	int z0,z1,z2,z3,z;
	Q hit0 = maintest(x,y,vx,vy);
	Q hit1, hit2, hit3;
	if(!hit0.yes) return nohit;
	if(side3 == NULL) sides -= 1;
	if(side2 == NULL) sides -= 1;
	if(side1 == NULL) sides -= 1;

	z0 = hit0.depth;
	if(side1) {hit1 = side1(x,y,vx,vy); z1 = hit1.depth;}
	if(side2) {hit2 = side2(x,y,vx,vy); z2 = hit2.depth;}
	if(side3) {hit3 = side3(x,y,vx,vy); z3 = hit3.depth;}

	if(sides == 0) return hit0;
	if(sides == 1){
		if(z0 < z1) return hit0;
		else return hit1;
	}
	if(sides == 2){
		z = min3(z0,z1,z2);
		if(z==z0) return hit0;
		if(z==z1) return hit1;
		if(z==z2) return hit2;
		printf("stage: general test is broken (code 2)\n");
		return nohit;
	}
	if(sides == 3){
		z = min4(z0,z1,z2,z3);
		if(z==z0) return hit0;
		if(z==z1) return hit1;
		if(z==z2) return hit2;
		if(z==z3) return hit3;
		printf("stage: general test is broken (code 0)\n");
		return nohit;
	}
	printf("stage: general test is broken (code 1)\n");
	return nohit;
}	



/*
test a single moving point for collision with the stage.
if a collision occurs the result will contain the collision
point, a new velocity, penetration depth, and surface type.
this data can be used in several ways by the client code.
testing moving rectangles is expected to use several calls.
this model has problems with sharp acute corners, so levels
should avoid them.
*/
struct stghit stage_collide(int x, int y, int vx, int vy){
	int tx = DDD(x) / 16;
	int ty = DDD(y) / 16;
	int x_ = x % L;
	int y_ = y % L;
	int W = this_stage->w;
	tile* t = this_stage->tiles + tx + ty*W;
	int mt;
	int sc;
	test sides[3];
	Q hit;

	unpack_config(t->config, &mt, &sc);
	if(maintests[mt] == NULL) mt = 1;
	get_sidetests(sc, sides);
	hit = general_test(x_, y_, vx, vy, maintests[mt],
		sides[0], sides[1], sides[2]);

	if(hit.yes){
		hit.x += UUU(tx * 16);
		hit.y += UUU(ty * 16);
	}
	return hit;
}


/* i believe this only works for rectangles strictly smaller than
 * a tile. 'works' means level should avoid sharp points.
 */
struct stghit stage_collide_rect(int x, int y, int w, int h, int vx, int vy){
	struct stghit hit0, hit1, hit2, hit3;
	int z0,z1,z2,z3,z;
//	struct stghit hit4, hit5, hit6, hit7;
//	int z4,z5,z6,z7;

	hit0 = stage_collide(x,y,vx,vy);
	hit1 = stage_collide(x+w,y,vx,vy);
	hit2 = stage_collide(x,y+h,vx,vy);
	hit3 = stage_collide(x+w,y+h,vx,vy);

	z0 = hit0.yes ? hit0.depth : INT_MIN;
	z1 = hit1.yes ? hit1.depth : INT_MIN;
	z2 = hit2.yes ? hit2.depth : INT_MIN;
	z3 = hit3.yes ? hit3.depth : INT_MIN;

	z = max4(z0,z1,z2,z3);

	if(z == z0) return hit0;
	if(z == z1){
		hit1.x -= w;
		return hit1;
	}
	if(z == z2){
		hit2.y -= h;
		return hit2;
	}
	if(z == z3){
		hit3.x -= w;
		hit3.y -= h;
		return hit3;
	}

	return nohit;
}


/* end collision stuff */






static void initialize_tiles(int n, tile* tiles){
	int i;
	for(i=0; i<n; i++){
		tiles[i].shape = SHAPE_FREE;
		tiles[i].fg = 0;
		tiles[i].bg = 0;
		tiles[i].config = 0;
	}
}

static void fix_tile_sides(stage* s){
	
}

static unsigned char base_config(enum tile_shape sh){
	/*    2
	 * 1     4
	 *    8
	 */
	switch(sh){
		case SHAPE_FREE: return pack_config(0, 0);
		case SHAPE_SQUARE: return pack_config(1, 0);

		case SHAPE_TRI_NE: return pack_config(2, 2|4);
		case SHAPE_TRI_NW: return pack_config(3, 1|2);
		case SHAPE_TRI_SE: return pack_config(4, 4|8);
		case SHAPE_TRI_SW: return pack_config(5, 1|8);

		case SHAPE_LTRAP_FLOOR: return pack_config(6, 1|8|4);
		case SHAPE_RTRAP_FLOOR: return pack_config(7, 1|8|4);
		case SHAPE_LSLOPE_FLOOR: return pack_config(8, 1|8); 
		case SHAPE_RSLOPE_FLOOR: return pack_config(9, 8|4);
		case SHAPE_LTRAP_CEIL: return pack_config(10, 1|2|4);
		case SHAPE_RTRAP_CEIL: return pack_config(11, 1|2|4);
		case SHAPE_LSLOPE_CEIL: return pack_config(12, 1|2);
		case SHAPE_RSLOPE_CEIL: return pack_config(13, 2|4);

		case SHAPE_HALF_FLOOR: return pack_config(14, 1|8|4);
		case SHAPE_HALF_CEIL: return pack_config(15, 1|2|4);
		default: return 0;
	}
}

static int load_stage(char* gfxpath, char* stagepath){
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

	reader* f = loader_open(stagepath);
	char* filename = path_ending(stagepath);
	char buf[256];
	char gfx[256];
	tile* tptr;
	stage* s = malloc(sizeof(stage));
	int w = 20;
	int h = 15;
	int x, y, ox, oy, fg, bg, shape;
	int i, j;

	loader_scanline(f, "%d %d %d %d", &w, &h, &ox, &oy);

	loader_scanline(f, "%s", buf);
	strcpy(gfx, gfxpath);
	strcat(gfx, buf);
	s->bgimage = load_bitmap(gfx);

	loader_scanline(f, "%s", buf);
	strcpy(gfx, gfxpath);
	strcat(gfx, buf);
	s->bgtiles = load_bitmap(gfx);

	loader_scanline(f, "%s", buf);
	strcpy(gfx, gfxpath);
	strcat(gfx, buf);
	s->fgtiles = load_bitmap(gfx);

	s->tiles = malloc(w*h*sizeof(tile));
	initialize_tiles(w*h, s->tiles);
	s->w = w;
	s->h = h;
	s->ox = ox;
	s->oy = oy;
	strncpy(s->id, filename, 256);
	s->id[255] = '\0';

	while(!loader_feof(f)){
		loader_scanline(f, "%d %d %d %d %c", &x, &y, &fg, &bg, &shape);
		tptr = s->tiles + (x+ox) + (s->w * (y+oy));
		tptr->fg = fg;
		tptr->bg = bg;
		tptr->shape = shape;
		tptr->config = base_config(shape);
	}

	fix_tile_sides(s);

	s->next = stages;
	stages = s;

	loader_close(f);
	return 0;
}

/***/


int load_zone(string name){
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
	char stagesdir[256] = "";
	char gfxpath[256] = "";
	char* stagepath;

	strcat(stagesdir, "zones/");
	strcat(stagesdir, name);
	strcat(stagesdir, "/stages/");

	strcat(gfxpath, "zones/");
	strcat(gfxpath, name);
	strcat(gfxpath, "/gfx/");

	strncpy(zone_name, name, 256);
	zone_name[255] = '\0';

	dirs = loader_readdir((string)stagesdir);
	if(dirs == NULL){
		printf("ERROR cant read dirs\n");
		return -1;
	}

	ptr = dirs->next;
	while(ptr){
		stagepath = ptr->item;
		if(load_stage(gfxpath, stagepath) < 0){
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

	strcpy(zone_name, "NO ZONE");
}

void switch_stage(string id){
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


static void draw_tiles(int gfx, char layer, int cx, int cy){
	tile* tbase = this_stage->tiles;
	tile* t;
	int tw = this_stage->w;
	int th = this_stage->h;
	int id;
	int gx, gy;

	int i;
	int j;
	int io = cx/16;
	int jo = cy/16;
	int i_ = io + 20;
	int j_ = jo + 15;

	int extra_x = 0;
	int extra_y = 0;
	int W, H;
	screen_dimensions(&W, &H);
	if(W>320){
		extra_x = (W - 320)/16/2 + 2;
	}
	if(H>240){
		extra_y = (H - 240)/16/2 + 2;
	}

	io -= extra_x;
	jo -= extra_y;
	i_ += extra_x;
	j_ += extra_y+1;

	if(io > tw-1) return;
	if(jo > th-1) return;

	if(io < 0) io = 0;
	if(jo < 0) jo = 0;
	if(i_ > tw) i_ = tw;
	if(j_ > th) j_ = th;

	for(j=jo; j<j_; j++){
		for(i=io; i<i_; i++){
			t = tbase + i + tw*j;
			if(layer=='b') id = t->bg;
			else if(layer=='f') id = t->fg;
			else id = 0;
			gx = 16*(id%16);
			gy = 16*(id/16);
			draw_gfx(gfx, i*16-cx, j*16-cy, gx, gy, 16, 16);
		}
	}
}

/*
cx, cy is the camera position
x, y i think is the offset from 0,0 you should shift
w, h i think is the width and height of the screen
*/
void stage_draw_bg(int cx, int cy){
	int i;
	int bw, bh, screen_w, screen_h;
	int bgimage;
	int bgshift;
	int parallax = 2;

	if(this_stage == NULL) return;

	/* draw background vertically centered
	tile horizontally at fraction of camera x */
	bgimage = this_stage->bgimage;
	gfx_dimensions(bgimage, &bw, &bh);
	screen_dimensions(&screen_w, &screen_h);
	bgshift = cx/parallax/bw;
	for(i=-1; i<(screen_w/bw)+2; i++){
		draw_gfx_raw(
			bgimage,
			(i+bgshift)*bw-cx/parallax,
			(screen_h-bh)/2,
			0, 0, bw, bh
		);
	}

	draw_tiles(this_stage->bgtiles, 'b', cx, cy);
}

void stage_draw_fg(int cx, int cy){
	draw_tiles(this_stage->bgtiles, 'f', cx, cy);
}


void stage_init(){
	int i;
	for(i=0; i<14; i++){
		maintests[i] = NULL;
	}

	sidetests[0] = st_left;
	sidetests[1] = st_right;
	sidetests[2] = st_top;
	sidetests[3] = st_bottom;

	maintests[0] = nulltest;
	maintests[1] = mt_square;
	maintests[2] = mt_triangle_ne;
	maintests[3] = mt_triangle_nw;
	maintests[4] = mt_triangle_se;
	maintests[5] = mt_triangle_sw;
	maintests[6] = mt_ltrap_floor;
	maintests[7] = mt_rtrap_floor;
	maintests[8] = mt_lslope_floor;
	maintests[9] = mt_rslope_floor;
	maintests[10] = mt_ltrap_ceil;
	maintests[11] = mt_rtrap_ceil;
	maintests[12] = mt_lslope_ceil;
	maintests[13] = mt_rslope_ceil;
	maintests[14] = mt_half_floor;
	maintests[15] = mt_half_ceil;
}



/*** debug ***/

void stage_debug(){
	stage* ptr = stages;
	int i, j;
	char c;

	printf("stage debug:\n\n");
	printf("zone: %s\n\n", zone_name);

	while(ptr){
		printf("stage: %s\n", ptr->id);
		printf("size: %dx%d\n", ptr->w, ptr->h);
		printf("origin: (%d, %d)\n", ptr->ox, ptr->oy);
		printf("bgimage: %d\n", ptr->bgimage);
		printf("bgtiles: %d\n", ptr->bgtiles);
		printf("fgtiles: %d\n", ptr->fgtiles);
		printf("tiles:\n");

		for(j=0; j<ptr->h; j++){
			for(i=0; i<ptr->w; i++){
				c = (ptr->tiles + i + ptr->w*j)->fg;
				/*
				if(isprint(c)) printf("%d", c);
				else printf("?");*/
				printf("[%02x]", c);
			}
			printf("\n");
		}


		printf("\n");
		ptr = ptr->next;
	}
}
