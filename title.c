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

#include "util.h"
#include "backend.h"
#include "graphics.h"
#include "game.h"

#include "title.h"





void title_keydown(int key){
  printf("you pressed key %d\n",key);
  if(key == ESCAPE_KEY){
    end_program();
  }
}

void title_keyup(int key){
printf("you release key %d\n",key);
}

void title_joymovex(int joy, int x){
printf("you moved joystick %d x axis to %d\n",joy,x);
}

void title_joymovey(int joy, int y){
printf("you moved joystick %d y axis to %d\n",joy,y);
}

void title_joypress(int joy, int button){
printf("you pressed joystick %d button %d\n",joy,button);
}

void title_joyrelease(int joy, int button){
printf("you released joystick %d button %d\n",joy,button);
}

void title_update(){

}

void title_draw(){

}

struct handler title_handler = {
title_keydown,title_keyup,title_joymovex,
title_joymovey,title_joypress,title_joyrelease
};


void title_setup(){
  //load some graphics
  //place the graphics
  //if any, setup sprite update callbacks
  set_handler(title_handler);
  game.update = title_update;
  game.draw = title_draw;

  printf("you just entered the title screen\n");
}