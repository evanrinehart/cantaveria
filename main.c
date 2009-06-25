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

#include "game.h"
#include "backend.h"
#include "intro.h"
#include "loader.h"

void update(){

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
    if(game.end){break;}
    delay(DELAY_AMOUNT);
  }
}

int main(int argc, char* argv[]){

  backend_init(argc, argv);
  loader_init("cv.data");

  intro_setup();
  main_loop();

  loader_quit();
  backend_quit();

  return 0;

}

