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

#include "sound.h"
#include "util.h"

int sample_rate;

float* sounds[64];
int id_ptr = 0;

void process_audio(float lout[], float rout[], int len){
  for(int i=0; i<len; i++){
    lout[i] = 0;
    rout[i] = 0;
  }
}

int load_sound(char* filename){
  
}

void play_sound(int id){
  sounds[];
}




int sound_table[64][8] = {
{10000,0,0,0,0,0,0,0},
{5000,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0},
};




short builder1(int N, int i){
  return 0;
}

short builder2(int N, int i){
  return 0;
}

short (*(sound_builders[5]))(int N, int i) = {
  builder1,
  builder2
};

void init_sound(int rate){
  sample_rate = rate;

  /* generate sounds */
  int S = rate/22050;

  for(int N=0; N<2; N++){
    int L = sound_table[N][0];
    sounds[N] = xmalloc(L*S*sizeof(float));
    for(int i=0; i<L; i++){
      sounds[N][i] = sound_builders[N](N, i);
    }
  }
}
