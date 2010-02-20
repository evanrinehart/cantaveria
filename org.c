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
#define RELEASE_STEPS (SAMPLE_RATE/333)
#define RELEASE_RATE 0.999
float sine_table[TABLE_SIZE];

int note2tablestep(float note){
	/* critical formula for tuning a table */
	/* table steps for 2pif/SAMPLE_RATE radians is...*/
	/* TABLE_SIZE/PI2 == entries per radian */
	/* step == 2pif/SAMPLE_RATE * (TABLE_SIZE/PI2) */
	/*      == f * TABLE_SIZE / SAMPLE_RATE */
	/*      == f_0 2^(note/12) TABLE_SIZE / SAMPLE_RATE */
	float f = 440 * pow(2, note/12.0);
	return TABLE_SIZE*f / SAMPLE_RATE;
}



float normalize(float note){
	/* original
	-inf, -24 : 4
	-24, -12: 2
	-12, 24: 1
	24,36: 0.5
	36,48: 0.25
	48, +inf: 0.125*/
	const float min = -48;
	const float max = 48;
	const float left = 4;
	const float right = 0.125;

	if(note < min) return left;
	if(note > max) return right;

	float ans = (note - min)*((right-left) / (max-min)) + left;
	if(ans > 4) return 4;
	if(ans < 0.125) return 0.125;
	return ans;
}



/* ORG_DEFAULT: default fallback instrument */
struct defstate {
	int on[16];
	int note[16];
	int step[16];
	int ptr[16];
	float release[16];
	int relptr[16];
	int bendstep[16];
	float bend;
};

void default_gen(struct defstate* data, int z, float out[], int count){
	if(data->on[z] == 0) return;
	int i;
	for(i=0; i<count; i++){
		int step = data->step[z] + data->bendstep[z];
		if(data->on[z] == 2){
			data->release[z] *= RELEASE_RATE;
			if(data->release[z] < 0.01){
				data->on[z] = 0;
				break;
			}
		}

		float factor = normalize(data->note[z] + data->bend) * data->release[z];
		float amp = sine_table[data->ptr[z]];
		out[i] += amp * factor;
		data->ptr[z] += step;
		while(data->ptr[z] >= TABLE_SIZE){
			data->ptr[z] -= TABLE_SIZE;
		}
		while(data->ptr[z] < 0){
			data->ptr[z] += TABLE_SIZE;
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


void default_bend_gen(struct defstate* data, int i, float bend){
	if(data->on[i] == 0) return;
	int note = data->note[i];
	data->bendstep[i] = note2tablestep(note + bend) - data->step[i];
}

void default_turn_on(struct defstate* data, int note){
	int step = note2tablestep(note);
	int i;
	for(i=0; i<16; i++){
		if(data->on[i]==0){
			data->step[i] = step;
			data->note[i] = note;
			data->on[i] = 1;
			data->ptr[i] = 0;
			data->release[i] = 1.0;
			default_bend_gen(data, i, data->bend);
			return;
		}
	}
}

void default_turn_off(struct defstate* data, int note){
	int i;
	for(i=0; i<16; i++){
		if(data->note[i] == note && data->on[i] == 1){
			data->on[i] = 2;
			return;
		}
	}
}

void default_bend(struct defstate* data, int amount){
	int max = 16383;
	int relative = amount - max/2;
	float bend = 1.0*relative/max * 4;
	int i;
	data->bend = bend;
	for(i=0; i<16; i++){
		default_bend_gen(data, i, bend);
	}
}

void default_control(void* ud, int type, int val1, int val2, int val){
	struct defstate* data = ud;
	switch(type){
		case EV_NOTEON: default_turn_on(data, val1); break;
		case EV_NOTEOFF: default_turn_off(data, val1); break;
		case EV_PITCHBEND: default_bend(data, val); break;
	}
}

void default_cleanup(void* data){ free(data); }

instrument make_default(){
	instrument ins;
	struct defstate* data = malloc(sizeof(struct defstate));
	int i;
	for(i=0; i<16; i++){
		data->on[i] = 0;
		data->note[i] = 0;
		data->step[i] = 100;
		data->ptr[i] = 0;
		data->bendstep[i] = 0;
		data->bend = 0;
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




