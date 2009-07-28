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

   evanrinehart@gmail.com
*/

#include <stdio.h>
#include <stdlib.h>

#include <math.h>

typedef struct {
  int tick;
  unsigned char midi[3];
} seq[256];


static struct {
/*
specs for synth
*/


  /* synth state */

  /* note ons */
  int notes_c;
  struct {
    char chan;
    char note;
  } notes[32];

  /* sequencer */
  int tick;
  int seq_c;
  seq* song;

  int songs_c;
  seq* songs[64];

} my;




void init_synth(){
  my.notes_c = 0;
  my.tick = 0;

  my.seq_c = 0;
  my.song = NULL;
  my.songs_c = 0;
}

void synth_update(float lout[], float rout[], int count, int sample_rate){
  for(int i=0; i<count; i++){
    lout[i] = 0;
    rout[i] = 0;

    for(int j=0; j<my.notes_c; j++){
      float s = sin(0);
      lout[i] += s;
      rout[i] += s;

      /* t += dt; */
    }
  }
}

int load_song(char* filename){
  return -1;
}

