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

int camera_x;
int camera_y;

int screen_x;
int screen_y;

struct {
  int flag;
  int key;
  int timer;
} repeat;

int hold_fire;



void press_fire(){
  //if screen exists change tile
  //else create new screen
  zone* z = game.zones[game.current_zone];
  int si = game.si;
  int sj = game.sj;
  struct screen* scr = ZONE_LOOKUP(z,si,sj);

  if(!scr){
    scr = xmalloc(sizeof(struct screen));
    z->screens[si+z->w*sj] = scr;
  }

   scr->tiles[cursor_x-si*20][cursor_y-sj*15] = 33;
  
}

void cursor_control(int key){
  switch(key){
    case LEFT_KEY: cursor_x--; break;
    case RIGHT_KEY: cursor_x++; break;
    case UP_KEY: cursor_y--; break;
    case DOWN_KEY: cursor_y++; break;
  }

  if(cursor_x < 0){cursor_x = 0;}
  if(cursor_y < 0){cursor_y = 0;}

  /*check for crossing a zone exit*/

  if(cursor_x < camera_x){
    camera_x--;
    point_camera(camera_x*16, camera_y*16);
  }
  if(cursor_x >= camera_x+20){
    camera_x++;
    point_camera(camera_x*16, camera_y*16);
  }
  if(cursor_y < camera_y){
    camera_y--;
    point_camera(camera_x*16, camera_y*16);
  }
  if(cursor_y >= camera_y+15){
    camera_y++;
    point_camera(camera_x*16, camera_y*16);
  }

  screen_x = cursor_x/20;
  screen_y = cursor_y/15;
  if(cursor_x < 0){screen_x--;}
  if(cursor_y < 0){screen_y--;}
  game.si = screen_x;
  game.sj = screen_y;

  cursor->x = cursor_x*16;
  cursor->y = cursor_y*16;

  if(hold_fire) press_fire();
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
      cursor_control(key);
      break;
    case FIRE_KEY:
      hold_fire = 1;
      press_fire();
      break;
  }

}

void edit_keyup(int key){
  if(key == repeat.key){
    repeat.flag = 0;
  }
  if(key == FIRE_KEY) hold_fire = 0;
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


void edit_draw(){
  draw_stage();
  draw_sprites();
  printf_small(1,1,"zone: %s",game.zones[0]->name);
  printf_small(1,10,"screen: %d,%d",screen_x,screen_y);
  printf_small(1,19,"cursor: %d,%d",cursor_x,cursor_y);
  printf_small(1,28,"camera: %d,%d",camera_x,camera_y);
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

  load_game();

  game.handler = edit_handler;
  game.update = NULL;
  game.draw = edit_draw;

  char** list = loader_readdir("zones");
  for(int i=0; list[i]; i++){
    printf("loading zone \"%s\"\n",list[i]);
    load_zone(list[i]);
  }
  loader_freedirlist(list);

  enable_stage(1);

  cursor_x = 0;
  cursor_y = 0;
  load_sprite("box.spr",SPR_BOX);
  cursor = enable_sprite(SPR_BOX);

  camera_x = 0;
  camera_y = 0;
  point_camera(0,0);

  hold_fire = 0;
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

