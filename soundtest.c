#include <stdio.h>

#include <input.h>
#include <graphics.h>
#include <console.h>
#include <transfer.h>
#include <seq.h>
#include <org.h>

void play(int note){
	seq_instant(EV_NOTEON, 0, note, 0);
}

void stop(int note){
	seq_instant(EV_NOTEOFF, 0, note, 0);
}

static void press(input in){
	if(in.button == ESCAPE_KEY){
		game_is_over();
	}

	console_printf("%s", input_str(in));

	switch(in.button){
		case LEFT_BUTTON: play(0); break;
		case RIGHT_BUTTON: play(4); break;
		case DOWN_BUTTON: play(2); break;
		case FIRE_BUTTON: play(5); break;
		case JUMP_BUTTON: play(7); break;
		case SPECIAL_BUTTON: play(9); break;
		default: break;
	}
}

static void release(input in){
	switch(in.button){
		case LEFT_BUTTON: stop(0); break;
		case RIGHT_BUTTON: stop(4); break;
		case DOWN_BUTTON: stop(2); break;
		case FIRE_BUTTON: stop(5); break;
		case JUMP_BUTTON: stop(7); break;
		case SPECIAL_BUTTON: stop(9); break;
		default: break;
	}
}

static void update(){
	console_update();
}

static void draw(){
}


void setup_inner(){
	set_handler(update, draw, press, release);
	console_clear();
}
