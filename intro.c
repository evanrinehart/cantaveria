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
*/

#include <stdio.h>

#include <input.h>
#include <transfer.h>
#include <gameover.h>

#include <graphics.h>
#include <console.h>


static void press(input in){
	if(in.button == ESCAPE_KEY){
		game_is_over();
	}
	else{
		setup_title();
	}
}

static void release(input in){

}





static void update(){
}

static void draw(){

}

void setup_intro(){
	set_handler(update, draw, press, release);

	console_clear();
	console_printf("this is the intro");
	console_printf("press any key");
}

