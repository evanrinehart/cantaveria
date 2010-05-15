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

#include <input.h>
#include <graphics.h>

#include <transfer.h>

static struct {
	int gfx;
	int counter;
	int t1, t2, t3;
} my;


static void press(input in){
	setup_title();
}

static void release(input in){

}


static void update(){
	my.counter++;

	if(my.counter > my.t3){
		setup_intro();
	}
}

static void draw(){
	if(my.counter > my.t1 && my.counter < my.t2){
		draw_bitmap(my.gfx, (320-256)/2, 0);
	}
}


void setup_splash(){
	set_handler(update, draw, press, release);

	my.counter = 0;
	my.t1 = 100;
	my.t2 = 500;
	my.t3 = 600;
	my.gfx = load_bitmap("gfx/splash.tga");
}
