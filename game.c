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
#include "graphics.h"
#include "game.h"
#include "intro.h"

struct game game;

void load_game(){
  //game.update = intro_update;
  //game.handler = intro_handler;
  //game.draw = intro_draw;

  game.rng = pseed(0);



  game.player_x = 0;
  game.player_y = 0;
  game.si = 0;
  game.sj = 0;
  game.current_zone = NULL;
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

  z->name = xmalloc(strlen(filename)+1);
  strcpy(z->name, filename);

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
        for(int j=0; j<game.zone_count; j++){
          if(strcmp(game.zones[j]->name, name)==0){
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

    for(int j=0; j<15; j++){
      for(int i=0; i<20; i++){
        //tile.type = read_byte(rd);
        //tile.id = read_byte(rd);
        //scr->tiles[i][j] = tile;
        scr->tiles[i][j] = read_byte(rd);
      }
    }

    z->screens[x + z->w*y] = scr;

  }

  game.zones[game.zone_count++] = z;
}






/*
game model

each screen contains certain entities that may collide with other entities
and the stage structure
your bullets (ENT_PSHOT)
enemy bullets (ENT_ESHOT)
decorative animated sprites (ENT_EFFECT)
player characters (ENT_PLAYER)
non player characters (ENT_NPC)
enemies (ENT_ENEMY)
bosses (ENT_BOSS) (hmm the this isnt a good type)
moving platforms (ENT_PLAT)

an entity may be in more than one screen. it will only be updated once but will
but tested for collision with stuff in both screens. a test is done to see if it
left the screen so it can be removed.

the algorithm goes like

for each screen
  update non visited entities
  mark the entities as been visited
  as part of updating check for
    player colliding with the level
    player colliding with enemies
    player colliding with enemy shots
    player colliding with platforms
    enemies colliding with the level
    enemies colliding with player shots
    enemies colliding with platforms
    
*/


/* different model

  entities are not stored in screens
  entities are active or inactive
  active entities are updated and checked for collision with each other
  inactive entities become active when they get close enough
  active entities become inactive when they are far enough away
    and when they get into a certain state

*/


/* another model
  entities are contained in exactly one screen at a time
  they are either active or inactive.
  when you approach a screen, contained entities are activated

  shots, players, platforms, etc, do not need this sort of treatment
  so these types of entities should be called mobiles. shots dont live
  long enough to matter, players cant leave the current screen, platforms cant
  leave their home screen.
*/

/* coordinates

pixel coords - pixel count from global top left of world
absolute coords - pixel coords * 256
tile coords - pixel coords / 16

screen coords - i,j of screen starting from top left screen of zone
zone coords - screen coord of zone start from global top left
*/


int box_collision(struct box* B1, struct box* B2){
  int l1 = B1->x;
  int r1 = B1->x + B1->w;
  int l2 = B2->x;
  int r2 = B2->x + B2->w;
  int t1 = B1->y;
  int b1 = B1->y + B1->h;
  int t2 = B2->y;
  int b2 = B2->y + B2->h;

  if(r1 < l2 || l1 > r2 || b1 < t2 || t1 > b2)
    return 0;
  else 
    return 1;
}


void update_mobile_motion(mobile* m){
  m->x += m->vx * dt;
  m->y += m->vy * dt;
  m->box.x = m->x - m->bxoff;
  m->box.y = m->y - m->byoff;
  m->spr->x = (m->x / PIXUP) - m->xoff;
  m->spr->y = (m->y / PIXUP) - m->yoff;
}



