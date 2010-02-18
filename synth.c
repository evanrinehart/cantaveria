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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <synth.h>
#include <seq.h>
#include <list.h>

/*
timing stuff

ticks -   384 per beat     (tpb)
beats -   120 per minute   (bpm)
samples - 22050 per second (srate)

control happens on tick boundaries
control must take effect on nearest sample boundary

samples per tick = (srate*60) / (bpm*tpb)
which in the above example = 28 and 32760/46080

*/

typedef struct {
	void (*generate)(void* ud, float out[], int count);
	void (*cut)(void* ud);
	void* ud;
} generator;

typedef struct {
	float L, R, V;
	void* ud;
	void (*mix)(void* ud, float out[], int count);
	void (*control)(void* ud, int type, int val1, int val2, int val);
} channel;

int srate;
int bpm = 120;
int tpb = 384;

int serr = 0; // 1/1000 of a sample

int tick;
int terr = 0; // 1/(bpm*tpb) of a sample
int terrd = 46080; //bpm * tpb


channel channels[16];





void set_sample_rate(int x){ srate = x; }
void set_bpm(int x){ bpm = x; }





void dummy_mix(void* ud, float out[], int count){
}

void dummy_control(void* ud, int type, int val1, int val2, int val){
}

channel make_dummy_channel(){
	channel ch;

	ch.L = 1;
	ch.R = 1;
	ch.V = 1;

	ch.ud = NULL;
	ch.mix = dummy_mix;
	ch.control = dummy_control;

	return ch;
}



struct foo {
	int on;
	float t;
	float f;
};

void foo_mix(void* ud, float out[], int count){
	struct foo* foo = ud;
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

float note2f(int note){
	return 440*3.14159*2*pow(2, note/12.0)/44100;
}

void foo_control(void* ud, int type, int val1, int val2, int val){
	struct foo* foo = ud;
	switch(type){
		case 0: foo->on = 1; break;
		case 2: foo->on = 0; break;
		case 1: foo->f = note2f(val1); break;
	}
}

channel make_foo_channel(){
	channel ch = make_dummy_channel();
	ch.mix = foo_mix;
	ch.control = foo_control;

	struct foo* foo = malloc(sizeof(struct foo));

	foo->t = 0;
	foo->f = 0;
	foo->on = 1;

	ch.ud = foo;

	return ch;
}



void mix(channel* ch, float in[], float left[], float right[], int count){
	int i;
	for(i=0; i<count; i++){
		left[i] += in[i] * ch->L * ch->V;
		right[i] += in[i] * ch->R * ch->V;
	}
}

void zero(float buf[], int count){
	int i;
	for(i=0; i<count; i++){
		buf[i] = 0;
	}
}

void reduce(float buf[], int count, float factor){
	int i;
	for(i=0; i<count; i++){
		buf[i] /= factor;
	}
}

void generate(float left[], float right[], int count){
	float buf[4096];
	int i;
	float V = 1.0f;

	zero(left, count);
	zero(right, count);
	for(i=0; i<16; i++){
		channel* ch = &(channels[i]);
		zero(buf, count);
		ch->mix(ch->ud, buf, count);
		mix(ch, buf, left, right, count);
	}
	reduce(left, count, 16.0f/V);
	reduce(right, count, 16.0f/V);
}

void control(event* e){
	int chan = e->chan;
	int type = e->type;
	int val1 = e->val1;
	int val2 = e->val2;
	int val  = (e->val2 << 8) | e->val1;
	channel* ch = &(channels[chan]);
	ch->control(ch->ud, type, val1, val2, val);
}

void immediate_control(){
	event* e = seq_get_immediate();
	while(e){
		control(e);
		e = seq_get_immediate();
	}
}

void synth_generate(float left[], float right[], int samples){
	int i=0;

	immediate_control();

	for(;;){
		int next = seq_lookahead(samples);
		generate(left+i, right+i, next-i);
		i = next;
		if(i == samples) break;
		control(seq_get_event());
	};
//	seq_advance(samples);
}


void synth_init(){
	int i;
	printf("  synth: ... ");

	channels[0] = make_foo_channel();
	for(i=1; i<16; i++){
		channels[i] = make_dummy_channel();
	}

	//srate = sample_rate;

	printf("OK\n");
}
