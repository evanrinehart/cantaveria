#include <stdio.h>

#include <input.h>
#include <graphics.h>
#include <console.h>
#include <transfer.h>
#include <seq.h>

void play(int note){
	seq_instant(1, 0, note, 0);
}

static void press(input in){
	if(in.button == ESCAPE_KEY){
		game_is_over();
	}

	console_printf("%s", input_str(in));

	switch(in.button){
		case LEFT_BUTTON: play(0); break;
		case RIGHT_BUTTON: play(4); break;
		case UP_BUTTON: play(3); break;
		case DOWN_BUTTON: play(2); break;
		case FIRE_BUTTON: play(5); break;
		case JUMP_BUTTON: play(7); break;
		case SPECIAL_BUTTON: play(9); break;
		default: break;
	}

	seq_instant(0, 0, 0, 0);
}

static void release(input in){
	seq_instant(2, 0, 0, 0);
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
