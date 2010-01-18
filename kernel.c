#include <stdlib.h>

#include <video.h>
#include <audio.h>
#include <input.h>

#include <loader.h>
#include <graphics.h>
#include <text.h>

#include <util.h>

#include <kernel.h>

extern void splash_setup();

static struct {
	void (*update)();
	void (*draw)();
	void (*press)(input in);
	void (*release)(input in);
} handler;

void terminate(){
	loader_quit();
	audio_quit();
	video_quit();
};

void initialize(int argc, char* argv[]){
	video_init(argc, argv);
	audio_init();
	input_init("FIXME");
	loader_init();
	graphics_init();
	text_init();
	rand_reset(RANDOM_SEED);
	atexit(terminate);

	splash_setup();
}


void set_handler(
	void (*update)(),
	void (*draw)(),
	void (*press)(input in),
	void (*release)(input in)
){
	handler.update = update;
	handler.draw = draw;
	handler.press = press;
	handler.release = release;
}

void dispatch_input(){
	input in = get_input();
	while(in.type != NO_INPUT){
		if(in.type == BUTTON_PRESS){
			handler.press(in);
		}
		else if(in.type == BUTTON_RELEASE){
			handler.release(in);
		}
		else if(in.type == END_OF_PROGRAM){
			game_is_over();
		}
		in = get_input();
	}
}

void update(){
	dispatch_input();
	fps_update();
	console_clear();
	animate_sprites();
	handler.update();
}

void draw(){
	handler.draw();
	draw_final();
}


int game_over = 0;

void game_is_over(){
	game_over = 1;
}

int is_game_over(){
	return game_over;
}
