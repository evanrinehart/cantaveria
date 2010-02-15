#include <stdio.h>

#include <input.h>
#include <graphics.h>
#include <console.h>
#include <transfer.h>


static void press(input in){
	if(in.button == ESCAPE_KEY){
		game_is_over();
	}

	console_printf("%s", input_str(in));
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
}
