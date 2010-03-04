#include <stdio.h>

#include <list.h>
#include <input.h>
#include <graphics.h>
#include <console.h>
#include <transfer.h>
#include <seq.h>
#include <org.h>


int main_note = 0;

#define CHANNEL 1

void play(int note){
	seq_instant(EV_NOTEON, CHANNEL, note+main_note, 0);
}

void stop(int note){
	seq_instant(EV_NOTEOFF, CHANNEL, note+main_note, 0);
}

void bend(int amount){
	seq_instant(EV_PITCHBEND, CHANNEL, 0, amount);
}

int bend_amount = 64;
int bend_v = 0;

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
		case L_BUTTON: bend_v = -1; break;
		case R_BUTTON: bend_v = 1; break;
		case START_BUTTON: main_note-=12; break;
		case SELECT_BUTTON: main_note+=12; break;
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
		case L_BUTTON: bend_v = 0; break;
		case R_BUTTON: bend_v = 0; break;
		default: break;
	}
}

static void update(){
	console_update();

	bend_amount += bend_v;
	if(bend_amount > 0x7f) bend_amount = 0x7f;
	if(bend_amount < 0x00) bend_amount = 0x00;
	if(bend_v != 0){
		bend(bend_amount);
	}
}

static void draw(){
}


void setup_inner(){
	set_handler(update, draw, press, release);
	console_clear();
}
