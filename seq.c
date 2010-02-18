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

#include <seq.h>
#include <list.h>
#include <audio.h>
#include <util.h>

int tick;
int terr;
int loop_start;
int loop_end;
int looping;

//event lists
list* sequence;
list* blank_events;
list* immediate_events;

event* next_event;




void seq_init(){
	printf("  sequencer: ... ");

	blank_events = empty();
	immediate_events = empty();
	sequence = empty();

	printf("OK\n");
}

int would_loop(){
	return looping && next_event->tick > loop_end;
}

event* get_event_after(int tick){
	return NULL;
}

event* get_next_event(){
	if(would_loop()){
		return get_event_after(loop_start);
	}
	else {
		return next_event;
	}
}



int get_next_tick(){
	event* e = get_next_event();
	return e ? e->tick : -1;
}

int distance_to_next(){
	int next_tick = get_next_tick();
	if(next_tick < 0) return -1;

	return would_loop() ?
		next_tick - loop_start + loop_end - tick :
		next_tick - tick;

}


/* below are three functions that the synth uses to
control the sequencer. it finds control events, advances
the event pointer, and finally advances the tick count */

//returns samples from now an event will occur
//if no event will occur in sbound samples, returns -1
int seq_lookahead(int sbound){
return sbound;

	int tbound = sbound*46080/1323000;
	int T = distance_to_next();
	if(T < 0) return -1;
	return T > tbound ?
		sbound :
		T*1 + 0;
}

//returns the next event that would play
//if there is no such event, returns NULL
event* seq_get_event(){
return NULL;
	//needs to handle looping
}


//advance the tick position
void seq_advance(int samples){
return;
	// 46080 1/1323000 ticks = 1 sample
	int N = 46080 * samples;
	int D = 1323000;
	terr += N;
	tick += N/D + terr/D;
	terr %= D;
//needs to handle looping
}


/* IMPORTANT
it might well be simpler to implement looping as
an event which exists at the loop point after all
other events that occur at that time which sends
the sequence to a specific tick */




/* *** */
event* make_event(int type, int chan, int val1, int val2){
	event* e;
	if(blank_events->next){
		e = pop(blank_events);
	}
	else{
		e = xmalloc(sizeof(event));
	}

	e->type = type;
	e->chan = chan;
	e->val1= val1;
	e->val2= val2;
	e->tick = 0;
	return e;
}

void recycle_event(event* e){
	push(blank_events, e);
}


void seq_instant(int type, int chan, int val1, int val2){
	event* e = make_event(type, chan, val1, val2);
	audio_lock();
	append(immediate_events, e);
	audio_unlock();
}


void seq_play_sound(int id){
	seq_instant(0x90, 15, id, 0);
}



event* seq_get_immediate(){
	event* e = pop(immediate_events);
	if(e != NULL){
		recycle_event(e);
		return e;
	}
	else{
		return NULL;
	}
}
