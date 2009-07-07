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
#include <string.h>


#include "loader.h"
#include "util.h"
#include "backend.h"
#include "game.h"
#include "intro.h"

struct game game;


zone* zones[32];
int zone_count;

void load_game(){
  game.update = intro_update;
  game.handler = intro_handler;
  game.end = 0;

  game.rng = pseed(0);
}





/*
zones binary file format

[S] string
[b] byte
[s] short
[i] int
N[?] array
L[?] list

[S tileset]
256[b shape]
[s x] [s y] [s w] [s h]
L[ [s w] [s h]
  [b flags]
  300[b tile]
  4[S exit]  ]
*/

void load_zone(char* filename){

  reader* rd = data_open("zones/",filename);

  zone* z = xmalloc(sizeof(zone));
  struct screen* scr;
  //struct tile tile;

  z->name = filename;
  printf("loading zone \"%s\"\n",filename);

  char* str = read_string(rd);
  z->tileset_gfx = load_gfx(str);

  for(int i=0; i<256; i++){
    z->tileset_shapes[i] = read_byte(rd);
  }

  z->x = read_short(rd);
  z->y = read_short(rd);
  z->w = read_short(rd);
  z->h = read_short(rd);

  printf("zone dimensions {%d, %d, %d, %d}\n",z->x, z->y, z->w, z->h);

  z->screens = xmalloc(z->w * z->h * sizeof(struct screen*));
  for(int i=0; i<z->w*z->h; i++){
    z->screens[i] = NULL;
  }
  printf("allocated %dx%d = %d screen*\n",z->w, z->h, z->w*z->h); 

  int N = read_short(rd);
  printf("file contains %d screen definitions\n", N);
  for(int i=0; i<N; i++){
    scr = xmalloc(sizeof(struct screen));
    int x = read_short(rd);
    int y = read_short(rd);
    scr->flags = read_byte(rd);

    //skip decales for now

    for(int i=0; i<4; i++){
      char* name = read_string(rd);
      scr->exits[i] = -1;
      if(name){
        for(int j=0; j<zone_count; j++){
          if(strcmp(zones[j]->name, name)==0){
            scr->exits[i] = j;
          }
        }
      }
    }

    printf("screen %d ",i);
    printf("{x=%d, y=%d, flags=%x, exits[%d,%d,%d,%d]}\n",x,y,
      scr->flags,
      scr->exits[0],
      scr->exits[1],
      scr->exits[2],
      scr->exits[3]);

    for(int i=0; i<20; i++){
      for(int j=0; j<15; j++){
        //tile.type = read_byte(rd);
        //tile.id = read_byte(rd);
        //scr->tiles[i][j] = tile;
        scr->tiles[i][j] = read_byte(rd);
      }
    }

    z->screens[x + z->w*y] = scr;

  }

  zones[zone_count++] = z;
}

