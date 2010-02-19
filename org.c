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

#include <audio.h> /* to have SAMPLE_RATE */
#include <org.h>


instrument make_cool() {
	instrument ins;
	ins.mix = NULL;
	ins.control = NULL;
	ins.cleanup = NULL;
	ins.data = NULL;
	return ins;
}

float note2f(int note){
	return 440*3.14159*2*pow(2, note/12.0)/SAMPLE_RATE;
}

struct foo {
	int on;
	float t;
	float f;
};

void foo_mix(void* data, float out[], int count){
	struct foo* foo = data;
	int i;

	if(foo->on == 0) return;

	for(i=0; i<count; i++){
		out[i] += sin(foo->t);
		foo->t += foo->f;
		while(foo->t > 2*3.14159){
			foo->t -= 2*3.14159;
		}
	}
}

void foo_control(void* data, int type, int val1, int val2, int val){
	struct foo* foo = data;
	switch(type){
		case EV_NOTEON:
			foo->f = note2f(val1);
			foo->on = 1;
			break;
		case EV_NOTEOFF:
			if(foo->f == note2f(val1)){
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
