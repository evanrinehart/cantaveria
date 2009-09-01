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
#include <string.h>

#include <math.h>

#include "util.h"
#include "backend.h"
#include "loader.h"

typedef struct {
  int tick;
  unsigned char midi[3];
} event;

typedef struct {
  int len;
  event e[256];
} seq;




/* possible generators (scratch work)

square wave
saw wave
triangle wave
square saw produce a band limited signal

string signal
this uses a karplus strong algorithm
noise -> variable delay (linear interpolating) -> first order low pass

voice signal
formant synthesis
two or three gaussians (perhaps padded) -> IFFT
also works for bell

detuned oscillators

samples on ch 10
just samples

effects
portamento
vibrato
echo
chorus
reverb
appegiator
adsr envelope
*/


typedef struct {
  float t;
  float tt;
} instr_dummy;






static struct {

  /* synth state */
  struct {
    void* prog;
    int type;

    float vol;
    float pan;

    float f;
    
    struct {
      int en;
      float ftar;
      float speed;
    } porta;

    struct {
      int en;
      int counter;
      int counter_max;
      int delay;
      float depth;
    } vibra;

    struct {
      float A, D, S, R;
      float t;
      int state;
    } adsr;

    struct {
      int en;
      float* delay;
      int len;
      int ptr[8];
      int predelay;
      int time;
    } rev;

    struct {
      int en;
      float* delay;
      int len;
      int ptr;
      float decay;
    } echo;
    
  } chan[16];

  /* sample note ons */
  int sample_c;
  int sample[32];

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

    for(int j=0; j<16; j++){
      //process_chan(j, lout, rout, count);
    }
  }
}



void seq_append(seq* s, int tick, int type, int chan, int val1, int val2){
  if(s->len < 256){
    event* e = &(s->e[s->len++]);
    e->tick = tick;
    e->midi[0] = type|chan;
    e->midi[1] = val1;
    e->midi[2] = val2;
  }
}

int event_cmp(const void* p1, const void* p2){
  const event* e1 = p1;
  const event* e2 = p2;

  return e1->tick - e2->tick;
}

int get_delta(reader* f){
  int a = read_byte(f);
  if(a<0x80){return a;}

  int b = read_byte(f);
  if(b<0x80){return ((a&0x7f)<<7) | b;}

  int c = read_byte(f);
  if(c<0x80){return ((a&0x7f)<<14) | ((b&0x7f)<<7) | c;}

  int d = read_byte(f);
  return ((a&0x7f)<<21) | ((b&0x7f)<<14) | ((c&0x7f)<<7) | d;
}

int load_song(char* filename){

  reader* f = data_open("music", filename);

  seq* s = xmalloc(sizeof(seq));
  my.songs[my.songs_c] = s;

  char buf[16];
  char string[64];

  int uspq;
  int bpm;

  /*MThd*/
  loader_read(f,buf,4);

  /*0x00000006*/
  read_int(f);

  /*format type: 0x0000 0x0001 or 0x0002*/
  read_short(f);

  /*number of tracks*/
  short track_count = read_short(f);

  /* time division */
  loader_read(f,buf,2);
  //code to figure out time division

  for(int i=0; i<track_count; i++){

    /*MTrk*/
    loader_read(f,buf,4);

    /* chunk size */
    int chunk_size = read_int(f);
    printf("%d\n",chunk_size);

    int tick = 0;
    int end_of_track = 0;
    int last_type = 0x80;
    int last_chan = 0;
    while(1){
      int delta = get_delta(f);

      if(delta < 0) return -1;
      tick += delta;

      //type and channel
      buf[0] = read_byte(f);

      int type = buf[0] & 0xf0;
      if(type >= 0x80 && type <= 0xe0){//normal event
        last_type = type;
        int chan = buf[0] & 0x0f;
        last_chan = chan;
        loader_read(f,buf,2);
        int val1 = buf[0];
        int val2 = buf[1];
        seq_append(s, tick, type, chan, val1, val2);
      }
      else if(type < 0x80){//running status
        int val1 = buf[0];
        buf[0] = read_byte(f);
        int val2 = buf[0];
        seq_append(s, tick,last_type,last_chan,val1,val2);
      }
      else if(type == 0xff){//meta event
        buf[0] = read_byte(f);
        type = buf[0];

        int len = get_delta(f);

        switch(type){
          case 0x2f: printf("end of track\n");
            end_of_track = 1;
            break;
          case 0x51:printf("tempo change\n"); /*tempo*/
            loader_read(f,buf,3);
            uspq = (buf[0]<<16) | (buf[1]<<8) | buf[2];
            bpm = 120;/*FIXME*/
            break;
          case 0x01: printf("text\n");/*text*/
            if(len >= 64){/*too big, skip ahead*/
              loader_read(f, NULL, len);
            }
            else{
              loader_read(f,string,len);
              string[len] = '\0';
              if(strncmp(string,"LoopStart",len)==0){
                seq_append(s, tick, 0xf0, 0, 0, 0);
              }
              else if(strncmp(string,"LoopEnd",len)==0){
                seq_append(s, tick, 0xf0, 0, 1, 0);
              }
            }
            break;
          default: /*skip*/
            loader_read(f,NULL,len);
            break;
        }
      }
      else{ //sysex and such...
        int len = get_delta(f);
        loader_read(f, NULL, len);
      }

      if(end_of_track) break;
    }

  }


  qsort(s->e, s->len, sizeof(event), event_cmp);
  synth_setbpm(bpm);

  return my.songs_c++;
}






/* karplus strong algorithm */
/*
void karplus_strong(delayline* d, float out[], int count){
  delay_extract(d, 77.432, out, count);
  if(0){
    for(int i=0; i<50; i++){
      out[i] += randf()-0.5;
    }
  }
  for(int i=1; i<count; i++){
    out[i] = (out[i]+out[i-1])/2;
  }
  delay_write(d, out, count);
}*/
