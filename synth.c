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

#include <util.h>
#include <list.h>
#include <midi.h>
#include <seq.h>
#include <synth.h>
#include <orc.h>

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

int serr = 0; // 1/1000 of a sample

int tick;
int terr = 0; // 1/(bpm*tpb) of a sample
int terrd = 46080; //bpm * tpb


channel channels[16];










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
	instrument ins = orc_load(name);
	ch.mix = ins.mix;
	ch.control = ins.control;
	ch.cleanup = ins.cleanup;
	ch.data = ins.data;
	return ch;
}



void set_music_volume(int percent){

}

void cut_music(){

}

void fade_clear(){

}

void fadeout(int seconds){

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

void clip(float buf[], int count){
	int i;
	int clipped = 0;
	float avg = 0;
	for(i=0; i<count; i++){
		avg += buf[i]*buf[i];
		if(buf[i] > 1.0){
			clipped = 1;
			buf[i] = 1.0;
		}
		else if(buf[i] < -1.0) buf[i] = -1.0;
	}

	if(clipped){
		error_msg("synth: clipping distortion due to output overload\n");
	}

	avg /= count?count:1;
	avg = sqrt(avg);
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
	clip(left, count);
	clip(right, count);
}

void control(event* e){
	if(e == NULL) return;
	int chan = e->chan;
	int type = e->type;
	int val1 = e->val1;
	int val2 = e->val2;
	int val  = (e->val2 << 7) | e->val1;
	channel* ch = &(channels[chan]);

	switch(type){
		case EVX_MUSICVOLUME: set_music_volume(val1); break;
		case EVX_MUSICCUT: cut_music(); break;
		case EVX_FADECLEAR: fade_clear(); break;
		case EVX_FADEOUT: fadeout(val1); break;
		default: ch->control(ch->data, type, val1, val2, val); break;
	}
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
		if(e == NULL && used == 0){
			error_msg("synth: sequencer failed to advance or provide a control event.");
			break;
		}
	};
}


void synth_init(){
	int i;

	orc_init(SAMPLE_RATE);

	for(i=0; i<16; i++){
		channels[i] = make_dummy_channel();
	}

	//channels[0] = make_channel_from_instrument(ORC_KARPLUS);
	//channels[1] = make_channel_from_instrument(ORC_KARPLUS);
	//channels[2] = make_channel_from_instrument(ORC_KARPLUS);
	//channels[3] = make_channel_from_instrument(ORC_KARPLUS);
	channels[0] = make_channel_from_instrument(ORC_DEFAULT);
	channels[1] = make_channel_from_instrument(ORC_DEFAULT);
	channels[2] = make_channel_from_instrument(ORC_DEFAULT);
	channels[3] = make_channel_from_instrument(ORC_DEFAULT);

}
