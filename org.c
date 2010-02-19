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

/*
heres a list of instrument ideas...

piano (not sure yet)
brass (not sure yet)
string (not sure yet)
drum (probably samples)
bass kick (classic analog algorithm)
square triangle saw (polyblep ?)
wind instruments (additive synth via uninterpolated table)
sampler (perhaps a compressed array of samples)
rendered ambient (PADsynth)

*/

/* SINE TABLE */
#define TABLE_SIZE (1<<13)
float sine_table[TABLE_SIZE];

int note2tablestep(int note){
	/* critical formula for tuning a table */
	/* table steps for 2pif/SAMPLE_RATE radians is...*/
	/* TABLE_SIZE/PI2 == entries per radian */
	/* step == 2pif/SAMPLE_RATE * (TABLE_SIZE/PI2) */
	/*      == f * TABLE_SIZE / SAMPLE_RATE */
	/*      == f_0 2^(note/12) TABLE_SIZE / SAMPLE_RATE */
	return 440*pow(2, note/12.0)*TABLE_SIZE / SAMPLE_RATE;
}

float note2wavestep(int note){
	/* critical formula for tuning */
	/* delta(omega*t) == wave increment */
	/*                == 2 pi f / samp_rate */
	/*                == 2pi f_0 2^(note/12) / samp_rate */
	return 440*PI2*pow(2, note/12.0)/SAMPLE_RATE;
}




/* ORG_DEFAULT: default fallback instrument */
struct defstate {
	int on[16];
	int step[16];
	int ptr[16];
};

void default_gen(struct defstate* data, int z, float out[], int count){
	if(data->on[z] == 0) return;
	int i;
	for(i=0; i<count; i++){
		out[i] += sine_table[data->ptr[z]];
		data->ptr[z] += data->step[z];
		while(data->ptr[z] >= TABLE_SIZE){
			data->ptr[z] -= TABLE_SIZE;
		}
	}
}

void default_mix(void* ud, float out[], int count){
	struct defstate* data = ud;
	int i;
	for(i=0; i<16; i++){
		default_gen(data, i, out, count);
	}
}

void default_turn_on(struct defstate* data, int note){
	int step = note2tablestep(note);
	int i;
	for(i=0; i<16; i++){
		if(data->on[i]==0){
			data->step[i] = step;
			data->on[i] = 1;
			return;
		}
	}
}

void default_turn_off(struct defstate* data, int note){
	int step = note2tablestep(note);
	int i;
	for(i=0; i<16; i++){
		if(data->step[i] == step){
			data->on[i] = 0;
			return;
		}
	}
}

void default_control(void* ud, int type, int val1, int val2, int val){
	struct defstate* data = ud;
	switch(type){
		case EV_NOTEON: default_turn_on(data, val1); break;
		case EV_NOTEOFF: default_turn_off(data, val1); break;
	}
}

void default_cleanup(void* data){ free(data); }

instrument make_default(){
	instrument ins;
	struct defstate* data = malloc(sizeof(struct defstate));
	int i;
	for(i=0; i<16; i++){
		data->on[i] = 0;
		data->step[i] = 100;
		data->ptr[i] = 0;
	}
	ins.mix = default_mix;
	ins.control = default_control;
	ins.cleanup = default_cleanup;
	ins.data = data;
	return ins;
}







/*** exported methods ***/
instrument load_instrument(enum instrument_name name){
	switch(name){
		case ORG_DEFAULT: return make_default();
//		case ORG_COOL: return make_cool();
		default: return make_default();
	}
}


void org_init(){
	int i;
	for(i=0; i<TABLE_SIZE; i++){
		sine_table[i] = sin( (PI2*i) / TABLE_SIZE);
	}
}




