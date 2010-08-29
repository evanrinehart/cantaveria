#include <stdio.h>
#include <math.h>

#include <root.h>
#include <util.h>
#include <input.h>
#include <stage.h>
#include <entity.h>

static int update(Y* e){
	e->x = e->x + e->r.a;
	e->y = e->y + e->r.b;
	e->r.b += 10;
	return 0;
}

static int stage(Y* e, struct stghit hit){
	e->x = hit.x;
	e->y = hit.y;
/*	if(hit.surface == 0){
		e->r.a = hit.vx * 0.80;
		e->r.b = hit.vy * 0;
	}
	if(hit.surface == 1){
		e->r.a = hit.vx * 0.5;
		e->r.b = hit.vy;
	}
	if(hit.surface == 2){
		e->r.a = hit.vx;
		e->r.b = 0;
	}*/
	e->r.a = hit.vx;
	e->r.b = hit.vy * 0.1;
	return 0;
}

/* make an entity that moves in a circular path */
Y* mk_ent0(){
	Y* e = mk_dummy_ent(16*14*1024,16*3*1024);
	e->update = update;
	e->stage = stage;
	e->r.a = 0;
	e->r.b = 0;
	e->box.w = 8;
	e->box.h = 8;
	e->box.w *= 1024;
	e->box.h *= 1024;
	return e;
}


