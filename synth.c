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


int srate;
int bpm = 120;
int tpb = 384;

int serr = 0; // 1/1000 of a sample

int tick;
int terr = 0; // 1/(bpm*tpb) of a sample
int terrd = 46080; //bpm * tpb

list* channels;


typedef struct {
	int id;
	float L, R, V;
	void* ud;
	void (*mix)(void* ud, float out[], int count);
	void (*control)(void* ud, event* e);
} channel;


void set_sample_rate(int x){ srate = x; }
void set_bpm(int x){ bpm = x; }


void synth_init(int sample_rate){
	srate = sample_rate;
	channels = empty();
	seq_init();
}







channel* find_channel(int id){
	list* ptr = channels->next;
	while(ptr){
		channel* ch = ptr->item;
		if(ch->id == id) return ch;
		ptr = ptr->next;
	}
	return NULL;
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

void generate(float left[], float right[], int count){
	float buf[2048];
	list* ptr = channels->next;

	zero(left, count);
	zero(right, count);
	while(ptr){
		channel* ch = ptr->item;

		zero(buf, count);
		ch->mix(ch->ud, buf, count);
		mix(ch, buf, left, right, count);

		ptr = ptr->next;
	}
}

void control(event* e){
	channel* ch = find_channel(event_channel(e));
	ch->control(ch->ud, e);
}

void synth_generate(float left[], float right[], int samples){
	int i=0;
	for(;;){
		int next = seq_lookahead(samples);
		if(next < 0) break;
		generate(left+i, right+i, next-i);
		control(seq_get_event());
		i = next;
	};
	generate(left+i, right+i, samples-i);
	seq_advance(samples);	
}

