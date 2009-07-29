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

#include "util.h"
#include "backend.h"
#include "graphics.h"
#include "game.h"
#include "loader.h"

#include "title.h"
#include "intro.h"




void intro_keydown(int key){
  if(key == ESCAPE_KEY){
    end_program();
  }
  else{
    title_setup();
  }
}

void intro_keyup(int key){

}

void intro_joymovex(int joy, int x){

}

void intro_joymovey(int joy, int y){

}

void intro_joypress(int joy, int button){

}

void intro_joyrelease(int joy, int button){

}

struct handler intro_handler = {
intro_keydown,intro_keyup,intro_joymovex,
intro_joymovey,intro_joypress,intro_joyrelease
};





void intro_update(){
  console_printf("this is the intro");
  console_printf("press any key");
}

void intro_draw(){
  
}

void intro_setup(){
  set_handler(intro_handler);
  game.update = intro_update;
  game.draw = intro_draw;
}

