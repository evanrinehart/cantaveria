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
*/

#include <stdlib.h>

#include <SDL/SDL.h>

#include <synth.h>
#include <audio.h>
#include <util.h>

float* lout;
float* rout;

void audio_callback(void *userdata, Uint8 *stream, int bytes){
	int i, j;
	Sint16* out = (Sint16*)stream;
	int buflen = bytes / 2;        /* Sint16 = 2 bytes */
	int samples = buflen / 2;      /* 2 channels */

	synth_generate(lout, rout, samples);

	for(i=0, j=0; i<samples; i++){
		out[j] = (Sint16)(lout[i]*32767); j++;
		out[j] = (Sint16)(rout[i]*32767); j++;
	}
}


void audio_init(){
	SDL_AudioSpec want;
	SDL_AudioSpec got;


	want.freq = SAMPLE_RATE;
	want.format = AUDIO_S16;
	want.channels = 2;
	want.samples = BUFFER_SIZE;
	want.callback = audio_callback;


	if(SDL_OpenAudio(&want, &got)<0){
		fatal_error("sdl: cannot open audio (%s)\n", SDL_GetError());
	}

	printf("audio:\n");
	printf(" sample rate = %d\n", got.freq);
	printf(" channels = %d\n", got.channels);
	printf(" samples = %d\n", got.samples);
	printf(" format = %d\n", got.format);

	if(got.format != AUDIO_S16){
		printf("    WARNING: audio format not AUDIO_S16 :(\n");
		return;
	}

	lout = xmalloc(got.samples*sizeof(float));
	rout = xmalloc(got.samples*sizeof(float));

	printf(" sound on\n");
	SDL_PauseAudio(0);
	
}

void audio_quit(){
	SDL_CloseAudio();
	free(lout);
	free(rout);
}




