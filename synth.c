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

#include <list.h>
#include <org.h>
#include <synth.h>
#include <seq.h>

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
	float L, R, V;
	mix_callback mix;
	control_callback control;
	cleanup_callback cleanup;
	void* data;
} channel;

int srate;
int bpm = 120;
int tpb = 384;

int serr = 0; // 1/1000 of a sample

int tick;
int terr = 0; // 1/(bpm*tpb) of a sample
int terrd = 46080; //bpm * tpb


channel channels[16];





void set_bpm(int x){ bpm = x; }





void dummy_mix(void* v, float f[], int i){}
void dummy_control(void* v, int a, int b, int c, int d){}
void dummy_cleanup(void* v){}

channel make_dummy_channel(){
	channel ch;
	ch.L = 1;
	ch.R = 1;
	ch.V = 1;
	ch.mix = dummy_mix;
	ch.cleanup = dummy_cleanup;
	ch.control = dummy_control;
	ch.data = NULL;
	return ch;
}

channel make_channel_from_instrument(enum instrument_name name){
	channel ch = make_dummy_channel();
	instrument ins = load_instrument(name);
	ch.mix = ins.mix;
	ch.control = ins.control;
	ch.cleanup = ins.cleanup;
	ch.data = ins.data;
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
		ch->mix(ch->data, buf, count);
		mix(ch, buf, left, right, count);
	}
	reduce(left, count, 16.0f/V);
	reduce(right, count, 16.0f/V);
}

void control(event* e){
	if(e == NULL) return;
	int chan = e->chan;
	int type = e->type;
	int val1 = e->val1;
	int val2 = e->val2;
	int val  = (e->val2 << 7) | e->val1;
	channel* ch = &(channels[chan]);
	ch->control(ch->data, type, val1, val2, val);
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
	int remaining = samples;
	int used = 0;
	event* e = NULL;

	immediate_control();

	while(remaining > 0){
		e = seq_advance(remaining, &used);
		generate(left+i, right+i, used);
		control(e);
		i += used;
		remaining -= used;
		if(e == NULL && remaining > 0){
			printf("synth: sequencer error. no event was returned, but unable to advance beyond %d samples (out of %d)\n", i, samples);
			printf("synth: i will continue, but this caused a tick/sample drift of %d samples\n", remaining);
			break;
		}
	};
}


void synth_init(){
	int i;
	printf("  synth: ... ");

	channels[0] = make_channel_from_instrument(ORG_DEFAULT);
	channels[1] = make_channel_from_instrument(ORG_KARPLUS);
	for(i=2; i<16; i++){
		channels[i] = make_dummy_channel();
	}

	//srate = sample_rate;

	printf("OK\n");
}
