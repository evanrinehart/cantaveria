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
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <util.h>
#include <input.h>
#include <video.h>
#include <graphics.h>
#include <game.h>
#include <loader.h>

#include <title.h>
#include <intro.h>



void intro_press(input in){
	if(in.button == ESCAPE_KEY){
		end_program();
	}
	else{
		title_setup();
	}
}

void intro_release(input in){

}





void intro_update(){
	console_printf("this is the intro");
	console_printf("press any key");
}

void intro_draw(){

}

void intro_setup(){
	set_handler(
		intro_update, intro_draw,
		intro_press, intro_release
	);
}

