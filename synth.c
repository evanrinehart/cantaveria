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

#include <synth.h>
#include <seq.h>

/*
timing stuff

ticks -   384 per beat     (tpb)
beats -   120 per minute   (bpm)
samples - 22050 per second (srate)

control happens on tick boundaries
control must take effect on nearest sample boundary

samples per tick = (srate*60) / (bpm*tpb)
which in the above example = 28 and 32760/46080

*/


int srate;
int bpm = 120;
int tpb = 384;

int serr = 0; // 1/1000 of a sample

int tick;
int terr = 0; // 1/(bpm*tpb) of a sample
int terrd = 46080; //bpm * tpb

void set_sample_rate(int x){ srate = x; }
void set_bpm(int x){ bpm = x; }

void synth_init(int sample_rate){
	srate = sample_rate;
}

void generate(int samples, float left[], float right[]){
	int i;
	for(i=0; i<samples; i++){
		left[i] = 0;
		right[i] = 0;
	}
}

void control(event* e){

}

void synth_generate(float left[], float right[], int samples){
	int i=0;
	for(;;){
		int next = seq_lookahead(samples);
		if(next < 0) break;
		generate(next - i, left+i, right+i);
		control(seq_get_event());
		i = next;
	};
	generate(samples - i, left+i, right+i);
	seq_advance(samples);	
}













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
