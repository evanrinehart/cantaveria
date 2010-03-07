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
#include <util.h>
#include <audio.h>
#include <midi.h>
#include <seq.h>

int tick;
int terr;
int loop_start;
int enable = 0;

int bpm = 120;
int tpb = 384;

//event lists
list* sequence;
list* blank_events;
list* immediate_events;

list* next_event;
list* event_after_loop;



void advance_event(event* now){
	if(now->type == EVX_LOOPEND){
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

		if(e->type == EVX_TEMPOCHANGE){
//printf("tempo change to %d bpm\n", e->val1);
			bpm = e->val1;
		}

		if(e->type == EVX_TICKSPERBEAT){
//printf("setting tpb to %d\n", e->val1);
			tpb = e->val1;
		}

		if(e->type == EVX_LOOPSTART){
			loop_start = e->tick;
			event_after_loop = next_event;
		}


//printf("event (%d, %03x, %d, %d, %d)\n", e->tick, e->type, e->chan, e->val1, e->val2);

		return e;
	}
	else{
		return NULL;
	}
}

int samples_until_next(int max){
	int N = 2646000;
	int D = bpm*tpb;

	if(next_event == NULL) return max;

	event* e = next_event->item;
	if(e->tick - tick > max * D / N) return max;

	int d = (e->tick-tick)*N/D;
	return d < max ? d : max;
}

event* seq_advance(int max, int* used){
	int N = bpm*tpb;     // bpm * tpb
	int D = 2646000;   // srate * 60
	event* e;

	int u = samples_until_next(max);

	if(!enable){
		*used = max;
		return NULL;
	}

	*used = u;
	e = dequeue_event();

	terr += N * u;
	tick += terr/D;
	terr %= D;

	return e;
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
	blank_events = empty();
	immediate_events = empty();
	sequence = empty();

	loop_start = 0;

	next_event = sequence->next;
}

void seq_load(list* events){
	audio_lock();
		tick = 0;
		terr = 0;
		sequence = events;
		next_event = sequence->next;
		event_after_loop = next_event;
		loop_start = 0;
	audio_unlock();
}

void seq_seek(list* target){
	event* e = target->item;
	audio_lock();
		next_event = target;
		tick = e->tick;
	audio_unlock();
}

list* seq_tell(){
	return next_event;
}

void seq_enable(){
	enable = 1;
}

void seq_disable(){
	seq_instant(EVX_MUSICCUT, 0, 0, 0);
	enable = 0;
}


