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

#include <org.h>
#include <seq.h>
#include <synth.h>
#include <util.h>
#include <audio.h>

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



char* sample_format_str(int format){
	switch(format){
		case AUDIO_S16: return "signed 16-bit LE";
		case AUDIO_U16: return "unsigned 16-bit LE";
		case AUDIO_S16MSB: return "signed 16-bit BE";
		case AUDIO_U16MSB: return "unsigned 16-bit BE";
		case AUDIO_S8: return "signed 8-bit";
		case AUDIO_U8: return "unsigned 8-bit";
		default: return "unknown";
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
		report_error("sdl: cannot open audio (%s)\n", SDL_GetError());
		exit(-1);
	}

	printf("audio:\n");
	printf("  sample rate: %d\n", got.freq);
	printf("  channels: %d\n", got.channels);
	printf("  samples: %d\n", got.samples);
	printf("  format: %s\n", sample_format_str(got.format));

	if(got.format != AUDIO_S16){
		printf("    WARNING: audio format not AUDIO_S16 :(\n");
		SDL_CloseAudio();
		printf("  *no sound*\n");
		return;
	}
	lout = xmalloc(got.samples*sizeof(float));
	rout = xmalloc(got.samples*sizeof(float));
	memset(lout, 0, got.samples*sizeof(float));
	memset(rout, 0, got.samples*sizeof(float));

	org_init();
	synth_init();
	seq_init();

	printf("  sound on\n");
	SDL_PauseAudio(0);
	

}

void audio_quit(){
	SDL_CloseAudio();
	free(lout);
	free(rout);
}



void audio_lock(){
	SDL_LockAudio();
}

void audio_unlock(){
	SDL_UnlockAudio();
}
