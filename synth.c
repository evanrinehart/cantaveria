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
	void (*control)(void* ud, event* e);
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

void dummy_control(void* ud, event* e){
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

	zero(left, count);
	zero(right, count);
	for(i=0; i<16; i++){
		channel* ch = &(channels[i]);
		zero(buf, count);
		ch->mix(ch->ud, buf, count);
	return;
		mix(ch, buf, left, right, count);
	}
	reduce(left, count, 16.0f);
	reduce(right, count, 16.0f);
}

void control(event* e){
	int id = event_channel(e);
	channel* ch = &(channels[id]);
	ch->control(ch->ud, e);
}

void synth_generate(float left[], float right[], int samples){
	int i=0;
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

	for(i=0; i<16; i++){
		channels[i] = make_dummy_channel();
	}

	//srate = sample_rate;

	printf("OK\n");
}
