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
#include "game.h"
#include "backend.h"
#include "loader.h"
#include "graphics.h"
#include "text.h"

void update(){
  
}

void main_loop(){
  since();
  int T = 0;
  while(1){
    T += since();
    for(int i=0; i<T/dt; i++){
      input();
      update();
    }
    if(T/dt > 0){
      draw();
      T %= dt;
    }
    if(game.end){break;}
    delay(DELAY_AMOUNT);
  }
}


void edit_keydown(int key){
  printf("you pressed key %d\n",key);
  if(key == keynum(ESCAPE_KEY)){
    game.end = 1;
  }

  
}

void edit_keyup(int key){
printf("you release key %d\n",key);
}

void edit_joymovex(int joy, int x){
printf("you moved joystick %d x axis to %d\n",joy,x);
}

void edit_joymovey(int joy, int y){
printf("you moved joystick %d y axis to %d\n",joy,y);
}

void edit_joypress(int joy, int button){
printf("you pressed joystick %d button %d\n",joy,button);
}

void edit_joyrelease(int joy, int button){
printf("you released joystick %d button %d\n",joy,button);
}

struct handler edit_handler = {
  edit_keydown, edit_keyup, edit_joymovex,
  edit_joymovey, edit_joypress, edit_joyrelease
};

void main_init(int argc, char* argv[]){
  backend_init(argc, argv);
  loader_init();
  graphics_init();
  text_init();

  game.handler = edit_handler;
  game.update = NULL;

  enable_stage(1);
}

void main_quit(){
  loader_quit();
  backend_quit();
}

int main(int argc, char* argv[]){

  main_init(argc, argv);
  main_loop();
  main_quit();

  return 0;

}

