#include <stdio.h>

#include <util.h>
#include <gameover.h>
#include <input.h>
#include <graphics.h>
#include <console.h>
#include <transfer.h>
#include <list.h>

#include <music.h>
#include <audio.h>
#include <midi.h>
#include <seq.h>


int x = 0;

static void press(input in){
	if(in.button == ESCAPE_KEY){
		game_is_over();
	}

	if(in.button == FIRE_BUTTON){
    seq_play_sound(0);
		console_printf("peak %d dB, rms %d dB", audio_peak_level(), audio_rms_level());
	}

	if(in.button == UP_BUTTON){
		//increase volume
	}

	if(in.button == DOWN_BUTTON){
		//decrease volume
	}

/*
	if(x){
		console_printf("pausing music");
		music_pause();
		x = 0;
	}
	else{
		console_printf("playing music");
		music_play(MUS_TEST1);
		x = 1;
	}
*/
}

static void release(input in){
}

static void update(){
	console_update();
}

static void draw(){
}


void setup_inner(){
	set_handler(update, draw, press, release);
	console_clear();

  // write a sound effect for the synth
  // load a sound effect
  // setup a event (FIRE key) to play the sound effect for the test

/*
	if(music_load("music/last_battle.mid", MUS_TEST1) < 0){
		error_msg("open music failed\n");
	}
	else{
		music_play(MUS_TEST1);
	}
*/
}
