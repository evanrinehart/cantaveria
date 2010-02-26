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

#include <music.h>

mus_id current_id = MUS_NOTHING;

int music_load(char* filename, mus_id id){
	return -1;
}

void music_unload(mus_id id){

}

mus_id music_current(){
	return current_id;
}

void music_play(mus_id id){

}


void music_stop(mus_id id){

}

void music_pause(){

}

void music_volume(int precent){

}

void music_fadeout(int seconds){

}

