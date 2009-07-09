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



int cursor_x;
int cursor_y;
sprite* cursor;


struct {
  int flag;
  int key;
  int timer;
} repeat;




void cursor_control(int key){
  switch(key){
    case LEFT_KEY: cursor_x--; break;
    case RIGHT_KEY: cursor_x++; break;
    case UP_KEY: cursor_y--; break;
    case DOWN_KEY: cursor_y++; break;
  }

  cursor->x = cursor_x*16;
  cursor->y = cursor_y*16;
}

void edit_keydown(int key){

  if(key == ESCAPE_KEY){
    game.end = 1;
  }

  switch(key){
    case LEFT_KEY:
    case RIGHT_KEY:
    case UP_KEY:
    case DOWN_KEY:
      repeat.flag = 1;
      repeat.key = key;
      repeat.timer = 0;
  }

  cursor_control(key);
}

void edit_keyup(int key){
  if(key == repeat.key){
    repeat.flag = 0;
  }  
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


  char** list = loader_readdir("zones");
  for(int i=0; list[i]; i++){
    printf("loading zone \"%s\"\n",list[i]);
    load_zone(list[i]);
  }
  loader_freedirlist(list);

  enable_stage(1);

  cursor_x = 0;
  cursor_y = 0;
  int id = load_sprite("box.spr",SPR_BOX);
  cursor = enable_sprite(SPR_BOX);
}


void update(){
  if(repeat.flag){
    repeat.timer++;
    if(repeat.timer > 40){
      repeat.timer = 38;
      cursor_control(repeat.key);
    }
  }
}


void main_quit(){
  loader_quit();
  backend_quit();
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

int main(int argc, char* argv[]){

  main_init(argc, argv);
  main_loop();
  main_quit();

  return 0;

}

