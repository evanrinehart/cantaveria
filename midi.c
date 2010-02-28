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
#include <string.h>

#include <list.h>
#include <loader.h>
#include <seq.h>
#include <util.h>


static event* make_event(int tick, int type, int chan, int val1, int val2){
	event* e = malloc(sizeof(event));
	e->tick = tick;
	e->type = type;
	e->chan = chan;
	e->val1 = val1;
	e->val2 = val2;
	return e;
}

void free_events(list* events){
	list* ptr = events->next;

	while(ptr){
		free(ptr->item);
		ptr = ptr->next;
	}
}

int value_of_type(int type){
	switch(type){
		case 0x80: return -2;
		case EVX_LOOPSTART: return -1;
		case EVX_LOOPEND: return 1;
		case 0x90: return 2;
		default: return 0;
	}
}

int compare_event(void* v1, void* v2){
	event* e1 = v1;
	event* e2 = v2;
	int d1, d2;

	if(e1->tick < e2->tick) return 1;
	if(e1->tick > e2->tick) return -1;

	d1 = value_of_type(e1->type);
	d2 = value_of_type(e2->type);

	return d2 - d1;
}

/* read a midi variable length quantity */
/* returns number of bytes read or 0 for error */
/* fills in buf with bytes read */
int read_deltaenough(reader* r, unsigned char buf[4]){
	int bytes[4];
	int i;

	for(i=0; i<4; i++){
		if(read_byte(r, &bytes[i])) return 0;
		if(!(bytes[i] & 0x80)){
			buf[0] = bytes[0];
			buf[1] = bytes[1];
			buf[2] = bytes[2];
			buf[3] = bytes[3];
			return i+1;
		}
	}

	error_msg("midi_load: invalid delta encountered\n");
	return 0;
}

/* converts midi variable length quantity x int a normal quantity */
/* count should be number of bytes of data in x */
int conv_delta(unsigned char x[4], int count){
	switch(count){
	case 1: return x[0];
	case 2:	return ((x[0] & 0x7f) << 7) | x[1];
	case 3: return ((x[0] & 0x7f) << 14) | ((x[1] & 0x7f) << 7) | x[2];
	case 4: return ((x[0] & 0x7f) << 21) | ((x[1] & 0x7f) << 14) | ((x[2] & 0x7f) << 7) | x[3];
	default: printf("conv_delta: wrong count (%d)\n", count); return 0;
	}
}

/* read a midi variable length quantity from r */
/* 0 for success, -1 for error. out is filled in with quantity */
int read_delta(reader* r, int* out){
	unsigned char buf[4];
	int count = read_deltaenough(r, buf);

	if(count == 0){
		return -1;
	}
	else{
		*out = conv_delta(buf, count);
		return 0;
	}
}



event* meta_endoftrack(reader* r, int tick){
	int zero;
	read_delta(r, &zero);
	if(zero != 0){
		printf("end of track event not followed by zero\n");
	}
	return make_event(tick, EVX_ENDOFTRACK, 0, 0, 0);
}

event* meta_tempochange(reader* r, int tick){
	unsigned char bytes[3];
	int three;
	int uspqn;

	read_delta(r, &three);
	read_bytes(r, bytes, 3);
	uspqn = (bytes[0] << 16) | (bytes[1] << 8) | bytes[0];

	return make_event(tick, EVX_TEMPOCHANGE, 0, uspqn, 0);
}

event* meta_possibleloop(reader* r, int tick){
printf("ERR meta_possibleloop not done\n");
	return NULL;
}

event* meta_dummy(reader* r, int tick){
	int len;
	int foo;

	read_delta(r, &len);

	for(; len > 0; len--){
		read_byte(r, &foo);
	}

	return make_event(tick, EVX_META, 0, 0, 0);
}

event* skip_sys(reader* r, int tick){
printf("ERR skip_sys not done\n");
	return NULL;
}

event* read_normal(reader* r, int tick, int status){
	int type = status & 0xf0;
	int chan = status & 0x0f;
	int val1;
	int val2;

	if(read_byte(r, &val1) || read_byte(r, &val2))
		return NULL;
	else
		return make_event(tick, type, chan, val1, val2);
}

event* read_running(reader* r, int tick, int val1){
	int val2;

	if(read_byte(r, &val2))
		return NULL;
	else
		return make_event(tick, -1, -1, val1, val2);
}

event* read_meta(reader* r, int tick){
	int type;


	if(read_byte(r, &type))
		return NULL;

	switch(type){
		case 0x2f: return meta_endoftrack(r, tick);
		case 0x51: return meta_tempochange(r, tick);
		case 0x01: return meta_possibleloop(r, tick);
		default: return meta_dummy(r, tick);
	}
}


event* read_event(reader* r, int tick, int byte0){
	if(byte0 >= 0x80 && byte0 <  0xf0)
		return read_normal(r, tick, byte0);
	else if(byte0 < 0x80) /* not really the status! */
		return read_running(r, tick, byte0);
	else if(byte0 == 0xff)
		return read_meta(r, tick);
	else
		return skip_sys(r, tick);
}

int read_track(reader* r, list* events){
	int tick = 0;
	int last_type = 0x90;
	int last_chan = 0;

	unsigned char sig[4];
	unsigned chunk_size;

	if(read_bytes(r, sig, 4) || read_int(r, (int*)&chunk_size)){
		return -1;
	}

	if(memcmp(sig, "MTrk", 4) != 0){
		printf("MTrk not found\n");
		return -1;
	}

	for(;;){
		int delta;
		int byte0;
		if(read_delta(r, &delta) || read_byte(r, &byte0)){
			return -1;
		}

		tick += delta;

		event* e = read_event(r, tick, byte0);
		if(e == NULL){
			return -1;
		}

		append(events, e);

		if(e->type == -1){ /* running status */
			e->type = last_type;
			e->chan = last_chan;
		}
		else if(e->type == EVX_ENDOFTRACK){ /* end of track */
			break;
		}
		else{
		}
	}

	return 0;
}

int read_header(reader* r, int* format, int* track_count, int* divraw){
	char sig[4];
	int six;

	if(read_bytes(r, (unsigned char*)sig, 4)) return -1;
	if(read_int(r, &six)) return -1;
	if(read_short(r, format)) return -1;
	if(read_short(r, track_count)) return -1;
	if(read_short(r, divraw)) return -1;

	if(memcmp(sig, "MThd", 4) != 0){
		printf("MThd not found\n");
		return -1;
	}

	if(six != 6){
		printf("number six not found (%d)\n", six);
		return -1;
	}

	if(*format != 0 && *format != 1){
		printf("format not supported (%d)\n", *format);
		return -1;
	}

	return 0;
}

list* midi_load(char* filename){
	reader* r;
	list* events = empty();

	int format;
	int track_count;
	int divraw;
	int i;

	r = data_open("music/", filename);
	if(r == NULL){
		return NULL;
	}

	if(read_header(r, &format, &track_count, &divraw) < 0){
		error_msg("midi_load: read error in header\n");
		loader_close(r);
		recycle(events);
		return NULL;
	}

	//insert an special event about divraw
	//maybe fail if format == 2

	for(i=0; i<track_count; i++){
		if(read_track(r, events) < 0){
			error_msg("midi_load: read error in track\n");
			loader_close(r);
			free_events(events);
			recycle(events);
			return NULL;
		}
	}

	//sort(events, event_compare);
	return events;
}




