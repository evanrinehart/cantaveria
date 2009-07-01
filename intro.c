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
#include <math.h>

#include "game.h"
#include "backend.h"
#include "title.h"
#include "intro.h"


sprite* spr[4];

double t;

void intro_setup(){

  load_font("default.font");


  set_message("AB ウ DE");
  complete_message();

  
  game.update = intro_update;
  game.handler = intro_handler;
  game.end = 0;

  int m = load_sprite("sprite1.spr",0);

  for(int i=0; i<4; i++){
    spr[i] = enable_sprite(m);
  }
  spr[0]->x = 0;
  spr[0]->y = 0;
  spr[1]->x = 100;
  spr[1]->y = 0;
  spr[2]->x = 0;
  spr[2]->y = 100;
  spr[3]->x = 100;
  spr[3]->y = 100;
  //load some graphics
  //place the graphics
  //if any, setup sprite update callbacks

/*
  
*/

  printf("you just entered the intro\n");

}


void intro_keydown(int key){
  printf("you pressed key %d\n",key);
  if(key == keynum(ESCAPE_KEY)){
    title_setup();
    game.handler = title_handler;
    game.update = title_update;
  }
}

void intro_keyup(int key){
printf("you release key %d\n",key);
}

void intro_joymovex(int joy, int x){
printf("you moved joystick %d x axis to %d\n",joy,x);
}

void intro_joymovey(int joy, int y){
printf("you moved joystick %d y axis to %d\n",joy,y);
}

void intro_joypress(int joy, int button){
printf("you pressed joystick %d button %d\n",joy,button);
}

void intro_joyrelease(int joy, int button){
printf("you released joystick %d button %d\n",joy,button);
}


void intro_update(){
  for(int i=0; i<4; i++){
    spr[i]->x = 100*cos(t+i*3.14/2)+SCREEN_W/2-8;
    spr[i]->y = 100*sin(t+i*3.14/2)+SCREEN_H/2-8;
  }
  t += 0.001*dt;
}

struct handler intro_handler = {
intro_keydown,intro_keyup,intro_joymovex,
intro_joymovey,intro_joypress,intro_joyrelease
};


