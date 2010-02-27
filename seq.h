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




typedef struct event event;
struct event {
	int tick;
	int type;
	int chan;
	int val1;
	int val2;
};

void seq_init();

/* use these from audio thread */
event* seq_advance(int samples, int* used);
event* seq_get_immediate();

/***/
void seq_instant(int type, int chan, int val1, int val2);
void seq_clear();
void seq_load(list* events);
void seq_seek(int tick);
int seq_tell();
void seq_enable();
void seq_disable();
void seq_append(event* e);

