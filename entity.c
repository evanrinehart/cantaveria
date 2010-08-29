#include <stdlib.h>
#include <stdio.h>

#include <util.h>
#include <video.h>
#include <stage.h>
#include <input.h>
#include <entity.h>


extern Y* mk_ent0(void);

#define MAX_PLAYERS 4
#define MAX_ENTS 128

static Y* entities[MAX_ENTS];
static struct {int x, y;} ent_v[MAX_ENTS];
static int next_ent = MAX_PLAYERS;



int inside_rect(int x, int y, int x0, int y0, int x1, int y1){
	return !(x < x0 || x > x1 || y < y0 || y > y1);
}

int rects_overlap(rect r1, rect r2){
	return !(
		r1.x > r2.x + r2.w ||
		r2.x > r1.x + r1.w ||
		r1.y > r2.y + r2.h ||
		r2.y > r1.y + r1.h
	);
}



int update_noop(Y* e){
	return 0;
}

int stage_noop(Y* e, struct stghit hit){
	return 0;
}

int overlap_noop(Y* e, Y* f){
	return 0;
}

int diverge_noop(Y* e, Y* f){
	return 0;
}
static void find_visible_ents(int ents[], int* count){

}

void entity_master_simulate(){
	int visible_ents[MAX_ENTS];
	int visible_count = 0;

	find_visible_ents(visible_ents, &visible_count);
	
	{
		Y* e = entities[0];
		int x0 = e->x;
		int y0 = e->y;
		int vx;
		int vy;
		int w = e->box.w;
		int h = e->box.h;
		struct stghit hit;

		e->update(e);

		vx = e->x - x0;
		vy = e->y - y0;

		hit = stage_collide_rect(
			e->x, e->y,
			w, h,
			vx, vy
		);

		if(hit.yes){
			e->stage(e, hit);
		}
	}

	/* for all pairs, if overlapping do events */
	/* for all overlaps, if not overlapping do events */
	/* for all in screen, move and record v */
	/* for all in screen, stage collision */
}

void draw_entities(){
	int i;
	Y* e;
	for(i=0; i<MAX_ENTS; i++){
		if(entities[i] != NULL){
			e = entities[i];
			draw_gfx(0, e->x/1024, e->y/1024, 16, 0, 8, 8);
		}
	}
}

void setup_test_entities(){
	int i;
	for(i=0; i<MAX_ENTS; i++){
		entities[i] = NULL;
	}

	entities[0] = mk_ent0();
}

void player_press(int player, enum input_button button){
	Y* e = entities[0];
	switch(button){
		case LEFT_BUTTON: e->r.a = -1000; break;
		case RIGHT_BUTTON: e->r.a = 1000; break;
		case UP_BUTTON: e->r.b = -1000; break;
		case DOWN_BUTTON: e->r.b = 1000; break;
		case FIRE_BUTTON: stage_debug(); break;
		default: break;
	}
}

void player_release(int player, enum input_button button){
	Y* e = entities[0];
	switch(button){
		case LEFT_BUTTON: e->r.a = 0; break;
		case RIGHT_BUTTON: e->r.a = 0; break;
//		case UP_BUTTON: e->r.b = 0; break;
//		case DOWN_BUTTON: e->r.b = 0; break;
		default: break;
	}
}

/*
struct entity {
	int x; int y;  spatial
	int z;  z order 
	unsigned flbits;  common class 
	int gfx;  image 
	struct {int a,b,c,d,e,f;} r;
	struct {int x, y, w, h;} box; used to detect collision 
	int (*update)(Y*);
	int (*stage)(Y*,int,int);
	int (*overlap)(Y*,Y*);
	int (*diverge)(Y*,Y*);
};
*/

Y* mk_dummy_ent(int x, int y){
	memory clear_r = {0,0,0,0,0,0};
	rect standard = {0,0,16,16};
	Y* ent = malloc(sizeof(Y));
	ent->x = x;
	ent->y = y;
	ent->z = 0;
	ent->flbits = 0;
	ent->gfx = 0;
	ent->r = clear_r;
	ent->box = standard;
	ent->update = update_noop;
	ent->stage = stage_noop;
	ent->overlap = overlap_noop;
	ent->diverge = diverge_noop;
	return ent;
}




int mini_rand(int g){
	return (g*9 + 171) % 256;
}





