#include <stdio.h>

#include <input.h>
#include <graphics.h>
#include <console.h>
#include <transfer.h>
#include <list.h>

#include <music.h>


int x = 0;

static void press(input in){
	if(in.button == ESCAPE_KEY){
		game_is_over();
	}

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
		printf("open music worked\n");
		music_play(MUS_TEST1);
	}
}
