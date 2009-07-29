/*
   Cantaveria - action adventure platform game
   Copyright (C) 2009  Evan Rinehart

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
*/

#include <stdlib.h>

#include "synth.h"
#include "sound.h"
#include "util.h"


struct {
  float* sounds[64];
  int sound_c;
} my;

void process_audio(float lout[], float rout[], int len){
  for(int i=0; i<len; i++){
    lout[i] = 0;
    rout[i] = 0;
  }
}

int load_sound(char* filename){
  return 0;
}

void play_sound(int id){
  
}



void init_sound(){
  my.sound_c = 0;

  init_synth();
}




