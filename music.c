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
#include <music.h>
#include <seq.h>
#include <midi.h>
#include <util.h>

mus_id current_id = MUS_NOTHING;

list* songs[32] = {
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
};



list* positions[32];
int playing = 0;

static char* mus_name(mus_id id){
	switch(id){
		case MUS_NOTHING: return "MUS_NOTHING";
		case MUS_COOL: return "MUS_COOL";
		case MUS_TEST1: return "MUS_TEST1";
		default: return "(?)";
	}
}

static int is_id_invalid(mus_id id){
	if(id >= 32) return 1;
	else return 0;
}


int music_load(char* filename, mus_id id){
	list* events;

	if(id == MUS_NOTHING){
		printf("music_load: you can't load a song into MUS_NOTHING\n");
		return -1;
	}

	if(is_id_invalid(id)){
		printf("music_load: music id out of range (%d)\n", id);
		return -1;
	}

	if(songs[id] != NULL){
		printf("music_load: slot %s not empty\n", mus_name(id));
		return -1;
	}

	events = midi_load(filename);
	if(events == NULL){
		printf("music_load: unable to load \"%s\"\n", filename);
		return -1;
	}

	songs[id] = events;
	positions[id] = events->next;
	return 0;
}

void music_unload(mus_id id){
	if(is_id_invalid(id)) return;
	if(songs[id] == NULL) return;

	if(music_current() == id) music_stop(id);

	list* ptr = songs[id]->next;
	while(ptr){
		free(ptr->item);
		ptr = ptr->next;
	}

	recycle(songs[id]->next);
	songs[id] = NULL;
}

mus_id music_current(){
	return current_id;
}

void music_play(mus_id id){
	if(is_id_invalid(id)) return;
	if(songs[id] == NULL) return;
	if(music_current() == id && playing) return;

	if(playing) music_pause();
	seq_load(songs[id]);
	seq_seek(positions[id]);
	seq_enable();

	current_id = id;
	playing = 1;
}


void music_stop(mus_id id){
	if(is_id_invalid(id)) return;
	if(songs[id] == NULL) return;
	if(music_current() == id){
		seq_disable();
		playing = 0;
	}

	positions[id] = 0;
}

void music_pause(){
	playing = 0;
	seq_disable();
	positions[music_current()] = seq_tell();
}

void music_volume(int percent){
	seq_instant(EVX_MUSICVOLUME, 0, percent, 0);
}

void music_fadeout(int seconds){
	seq_instant(EVX_FADEOUT, 0, seconds, 0);
}

void music_print(mus_id id){
}

void music_debug(){
}

