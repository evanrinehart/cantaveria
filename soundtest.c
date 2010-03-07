#include <stdio.h>

#include <input.h>
#include <graphics.h>
#include <console.h>
#include <transfer.h>
#include <list.h>

#include <music.h>
#include <audio.h>


int x = 0;

static void press(input in){
	if(in.button == ESCAPE_KEY){
		game_is_over();
	}

	if(in.button == FIRE_BUTTON){
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

	if(music_load("last_battle.mid", MUS_TEST1) < 0){
		printf("open music failed\n");
	}
	else{
		music_play(MUS_TEST1);
	}
}
