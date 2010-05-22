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

enum {
	/* loader use */
	EVX_ENDOFTRACK = 0x100,
	EVX_META,

	/* seq use */
	EVX_TICKSPERBEAT,
	EVX_TEMPOCHANGE,
	EVX_LOOPSTART,
	EVX_LOOPEND,

	/* synth use */
	EVX_MUSICVOLUME,
	EVX_MUSICCUT,
	EVX_FADEOUT,
	EVX_FADECLEAR
};

typedef struct event event;
struct event {
	int tick;
	int type;
	int chan;
	int val1;
	int val2;
};

list* midi_load(string filename);

