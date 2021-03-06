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

   evanrinehart@gmail.com
*/
#include <stdlib.h>

#include <list.h>
#include <util.h>
#include <root.h>
#include <boot.h>

#include <video.h>
#include <input.h>

#include <console.h>
#include <graphics.h> // ?

#include <gameover.h>
#include <transfer.h>
#include <kernel.h>

int dt = QUANTUM; /* this is a global variable */

static struct {
	void (*update)();
	void (*draw)();
	void (*press)(input in);
	void (*release)(input in);
} handler;


static void terminate(){
	loader_quit();
	audio_quit();
	video_quit();
}

static void press(input in){ handler.press(in); }
static void release(input in){ handler.release(in); }

void dispatch_error(const char* msg){
	error_msg("kernel.c dispatch_input: %s\n", msg);
}

static void dispatch_input(){
	input in = get_input();
	while(in.type != NO_INPUT){
		switch(in.type){
			case BUTTON_PRESS: press(in); break;
			case BUTTON_RELEASE: release(in); break;
			case END_OF_PROGRAM: game_is_over(); break;
			case SKIP_INPUT: dispatch_error("SKIP_INPUT is not supposed to come from generator"); break;
			case INVALID_INPUT: dispatch_error("INVALID_INPUT produced in generator"); break;
			case NO_INPUT: dispatch_error("NO_INPUT is impossible here"); break;
		}
		in = get_input();
	}
}

void initialize(int argc, char* argv[]){
	video_init(argc, argv);
	seq_init();
	synth_init();
	audio_init();
	input_init("FIXME");
	loader_init();
	graphics_init();
	text_init();
	stage_init();
	rand_reset(RANDOM_SEED);

	atexit(terminate);

	setup_splash();
}

void update(){
	dispatch_input();
	fps_update();
//	console_clear();
	animate_sprites();
	handler.update();
}

void draw(){
	handler.draw();
	console_draw();
	draw_final();
}

void set_handler(
	void (*_update)(),
	void (*_draw)(),
	void (*_press)(input in),
	void (*_release)(input in)
){
	handler.update = _update;
	handler.draw = _draw;
	handler.press = _press;
	handler.release = _release;
}
