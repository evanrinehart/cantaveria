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

int positions[32];

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
	if(music_current() == id) return;

	music_stop(id);
	seq_load(songs[id]);
	seq_seek(positions[id]);
	seq_enable();

	current_id = id;
}


void music_stop(mus_id id){
	if(is_id_invalid(id)) return;
	if(songs[id] == NULL) return;
	if(music_current() == id) seq_disable();

	positions[id] = 0;
}

void music_pause(){
	seq_disable();
	positions[music_current()] = seq_tell();
}

void music_volume(int percent){
	/* somehow enqueue a special event */
}

void music_fadeout(int seconds){
	/* somehow enqueue a special event */
}

void music_print(mus_id id){
	list* ptr;

	if(is_id_invalid(id)){
		printf("%d is an invalid id\n", id);
		return;
	}

	if(songs[id] == NULL){
		char* name = mus_name(id);
		printf("%s is not loaded. use music_load(filename, %s)\n", name, name);
		return;
	}

	ptr = songs[id]->next;
	while(ptr){
		event* e = ptr->item;
		printf("(%d, %03x, %d, %d, %d)\n",
			e->tick,
			e->type,
			e->chan,
			e->val1,
			e->val2
		);
		ptr = ptr->next;
	}
}

void music_debug(){
	int i;
	printf("music:\n");
	for(i=0; i<32; i++){
		char* name = mus_name(i);
		int pos = positions[i];

		if(songs[i]){
			int count = length(songs[i]);
			printf("(%15s, %7de, %11d)\n", name, count, pos);
		}
		else{
			printf("(%15s, %7s, %11d)\n", name, "_", pos);
		}
	}
}

