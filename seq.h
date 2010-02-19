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


typedef enum {
	MUS_COOL


} mus_id;



typedef struct event event;
struct event {
	int tick;
	int type;
	int chan;
	int val1;
	int val2;
	struct event* next;
};

void seq_init();

/* use these from audio thread */
int seq_lookahead(int samples);
event* seq_get_event();
void seq_advance(int samples);


/* use these from main thread */
void music_play();
void music_pause();
void music_reset();
int music_load(char* filename, mus_id id);
void music_change(mus_id id);


/***/
void seq_play_sound(int id);
void seq_instant(int type, int chan, int val1, int val2);
event* seq_get_immediate();

