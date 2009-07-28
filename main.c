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
#include "loader.h"
#include "text.h"

#include "splash.h"



void update(){
  fps_update();
  console_clear();


  animate_sprites();
  game.update();


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
    if(ended()){break;}
    delay(DELAY_AMOUNT);
  }
}

void main_init(int argc, char* argv[]){
  backend_init(argc, argv);
  loader_init();
  graphics_init();
  text_init();
}

void main_quit(){
  loader_quit();
  backend_quit();
}

int main(int argc, char* argv[]){

  main_init(argc, argv);

  splash_setup();
  main_loop();

  main_quit();

  return 0;

}

