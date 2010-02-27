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

#include <list.h>
#include <seq.h>
#include <org.h>
#include <audio.h>
#include <util.h>

int tick;
int terr;
int loop_start;
int looping;

//event lists
list* sequence;
list* blank_events;
list* immediate_events;

list* next_event;
list* event_after_loop;



void advance_event(event* now){
	if(now->type == 0x100 && looping){
		next_event = event_after_loop;
		tick = loop_start;
	}
	else{
		next_event = next_event->next;
	}
}

event* dequeue_event(){
	if(next_event == NULL) return NULL;

	event* e = next_event->item;
	if(e->tick <= tick){
		advance_event(e);
		return e;
	}
	else{
		return NULL;
	}
}

int samples_until_next(int max){
	int N = 2646000;
	int D = 46080;

	if(next_event == NULL) return max;

	event* e = next_event->item;
	if(e->tick - tick > max * D / N) return max;

	int d = (e->tick-tick)*N/D;
	return d < max ? d : max;
}

event* seq_advance(int max, int* used){
	int N = 46080;     // bpm * tpb
	int D = 2646000;   // srate * 60

	int u = samples_until_next(max);

	terr += N * u;
	tick += terr/D;
	terr %= D;

	*used = u;
	return dequeue_event();
}




/* *** */
static event* make_event(int type, int chan, int val1, int val2){
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



void seq_enqueue(int when, int type, int chan, int val1, int val2){
	event* e = make_event(type, chan, val1, val2);
	e->tick = when;
	seq_append(e);
}

void seq_clear(){
	list* ptr = sequence->next;
	while(ptr){
		free(ptr->item);
		ptr = ptr->next;
	}
	recycle(sequence);
	sequence = empty();
}


void seq_append(event* e){
	append(sequence, e);
}

void seq_init(){
	printf("  sequencer: ... ");

	blank_events = empty();
	immediate_events = empty();
	sequence = empty();



	seq_enqueue(0, 0x90, 1, 0, 0);
	seq_enqueue(384, 0x80, 1, 0, 0);
	seq_enqueue(384, 0x90, 1, 2, 0);
	seq_enqueue(384*2, 0x80, 1, 2, 0);
	seq_enqueue(384*2, 0x90, 1, 4, 0);
	seq_enqueue(384*3, 0x80, 1, 4, 0);
	seq_enqueue(384*3, 0x90, 1, 0, 0);
	seq_enqueue(384*4, 0x80, 1, 0, 0);
	seq_enqueue(384*4, 0x90, 1, 0, 0);
	seq_enqueue(384*5, 0x80, 1, 0, 0);
	seq_enqueue(384*5, 0x90, 1, 2, 0);
	seq_enqueue(384*6, 0x80, 1, 2, 0);
	seq_enqueue(384*6, 0x90, 1, 4, 0);
	seq_enqueue(384*7, 0x80, 1, 4, 0);
	seq_enqueue(384*7, 0x90, 1, 0, 0);
	seq_enqueue(384*8, 0x80, 1, 0, 0);
	seq_enqueue(384*8, 0x100, 0, 0, 0);

	looping = 1;
	loop_start = 0;
	event_after_loop = sequence->next;

	next_event = sequence->next;
	printf("OK\n");
}

void seq_load(list* events){

}

void seq_seek(int tick){

}

int seq_tell(){
	return 0;
}

void seq_enable(){

}

void seq_disable(){

}

