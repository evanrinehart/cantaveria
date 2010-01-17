/*
   Cantaveria - action adventure platform game
   Copyright (C) 2009  Evan Rinehart

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

#include "util.h"
#include "backend.h"
#include "graphics.h"
#include "game.h"
#include "splash.h"
#include "intro.h"
#include "title.h"

static struct {
	int gfx;
	int counter;
	int t1, t2, t3;
} my;

void splash_keydown(int key){
	title_setup();
}
void splash_keyup(int key){}
void splash_joymovex(int joy, int x){}
void splash_joymovey(int joy, int y){}
void splash_joypress(int joy, int button){
	title_setup();
}
void splash_joyrelease(int joy, int button){}

struct handler splash_handler = {
	splash_keydown,splash_keyup,splash_joymovex,
	splash_joymovey,splash_joypress,splash_joyrelease
};

void splash_setup(){
	set_handler(splash_handler);
	game.update = splash_update;
	game.draw = splash_draw;

	my.counter = 0;
	my.t1 = 1000/dt;
	my.t2 = 5000/dt;
	my.t3 = 6000/dt;
	my.gfx = load_gfx("splash.tga");
}

void splash_update(){
	my.counter++;

	if(my.counter > my.t3){
		intro_setup();
	}
}

void splash_draw(){
	if(my.counter > my.t1 && my.counter < my.t2){
		draw_gfx(my.gfx,(320-256)/2,0,0,0,256,256);
	}
}


