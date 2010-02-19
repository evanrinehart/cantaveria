/*
org.c
various software synths and samples

Copyright (C) 2009 Evan Rinehart

This software comes with no warranty.
1. This software can be used for any purpose, good or evil.
2. Modifications must retain this license, at least in spirit.
*/

#include <stdlib.h>
#include <math.h>

#include <org.h>


instrument make_cool() {
	instrument ins;
	ins.mix = NULL;
	ins.control = NULL;
	ins.cleanup = NULL;
	ins.data = NULL;
	return ins;
}

float note2step(int note){
	/* critical formula for tuning */
	/* delta(omega*t) == wave increment */
	/*                == 2 pi f / samp_rate */
	/*                == 2pi f_0 2^(note/12) / samp_rate */
	return 440*PI2*pow(2, note/12.0)/SAMPLE_RATE;
}

struct foo {
	int on;
	float t;
	float step;
};

void foo_mix(void* data, float out[], int count){
	struct foo* foo = data;
	int i;

	if(foo->on == 0) return;

	for(i=0; i<count; i++){
		out[i] += sin(foo->t);
		foo->t += foo->step;
		while(foo->t > PI2){
			foo->t -= PI2;
		}
	}
}

void foo_control(void* data, int type, int val1, int val2, int val){
	struct foo* foo = data;
	switch(type){
		case EV_NOTEON:
			foo->step = note2step(val1);
			foo->on = 1;
			break;
		case EV_NOTEOFF:
			if(foo->step == note2step(val1)){
				foo->on = 0;
			}
			break;
	}
}

void foo_cleanup(void* data){ free(data); }

instrument make_foo(){
	instrument ins;
	ins.mix = foo_mix;
	ins.control = foo_control;
	ins.cleanup = foo_cleanup;
	ins.data = malloc(sizeof(struct foo));
	return ins;
}

instrument load_instrument(enum instrument_name name){
	switch(name){
		case ORG_FOO: return make_foo();
		case ORG_COOL: return make_cool();
		default: return make_foo();
	}
}
