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

#include "util.h"
#include "backend.h"


struct event {
  int tick;
  unsigned char midi[3];
};

typedef struct {
  struct event e[256];
} seq;

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
  int tpb;
  int bpm;
  int N;
  int N_;
  int R;
  int R_;
  int F;

  int seq_c;
  int seq_ptr;
  int seq_en;
  seq* song;

  int songs_c;
  seq* songs[64];

} my;



void synth_setbpm(int bpm){
  my.bpm = bpm;
  int numer = SAMPLE_RATE*60;
  int denom = my.tpb*bpm;
  int d = gcd(numer,denom);
  /*N and R exact only if at start of song, good enough though*/
  my.N = 0;
  my.N_ = numer/denom;
  my.R = 0;
  my.R_ = denom/d;
  my.F = (numer/d) % (denom/d);
}

void init_synth(){
  my.notes_c = 0;
  my.tick = 0;


  my.tpb = 288;
  synth_setbpm(120);


  my.seq_c = 0;
  my.seq_ptr = 0;
  my.songs_c = 0;
}

void eval_event(int i){
  unsigned char midi[3];
  midi[0] = my.song->e[i].midi[0];
  midi[1] = my.song->e[i].midi[1];
  midi[2] = my.song->e[i].midi[2];

  int type = midi[0]&0xf0;
  int chan = midi[0]&0x0f;
  int val1 = midi[1];
  int val2 = midi[2];

  switch(type){
    case 0x80: /* note off */
      printf("note off %x %d %d %d\n",type,chan+1,val1,val2);
      break;
    case 0x90: /* note on */
      printf("note off %x %d %d %d\n",type,chan+1,val1,val2);
      break;
    case 0xa0: /* aftertouch */
      break;
    case 0xb0: /* controller */
      break;
    case 0xc0: /* program */
      break;
    case 0xd0: /* channel pressure */
      break;
    case 0xe0: /* pitch bend */
      break;
    case 0xf0: /* custom non standard events (not sysex) */
      /* loop point, tempo change, etc */
      break;
  }
}

void synth_update(float lout[], float rout[], int count){
  
  for(int i=0; i<count; i++){
    lout[i] = 0;
    rout[i] = 0;


    if(my.seq_en){

      /* elaborate counter to keep ticks and samples in sync */
      /* N  sample counter */
      /* N_ integer samples per tick */
      /* R  fractional sample counter, units 1/R_ samples */
      /* R_ F/R_ samples is difference between real samples per tick and N_*/
      /* F  see above */
      my.N++; /* sample counter advance */
      if(my.N == my.N_){ /* almost at the next tick boundary */
        my.tick++; /* next tick comes F/R_ samples early */
        my.N=0;

        my.R+=my.F; /* add F/R_ */
        if(my.R >= my.R_){ /* enough fractional samples accumulated */
          my.R -= my.R_; /* fraction counter wraps */
          my.N++; /* advance extra sample */
        }
      }

      /* do next events maybe */
      while(my.tick == my.song->e[my.seq_ptr].tick){
        eval_event(my.seq_ptr++);
        if(my.seq_ptr==my.seq_c){
         my.seq_en = 0;
        }
      }

    }

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

