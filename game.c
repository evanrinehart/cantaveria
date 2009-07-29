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
#include <stdlib.h>

#include "loader.h"
#include "util.h"
#include "backend.h"
#include "graphics.h"
#include "game.h"
#include "intro.h"

struct game game;




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
L[ [s i] [s j]
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
  free(str);

  //str = read_string(rd);
  z->bg_gfx = load_gfx("background.tga");

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


enum {
GROUND,
JUMPING,
FALLING
};

char statenames[8][16] = {
"GROUND",
"JUMPING",
"FALLING"
};

struct pstate {
  int state;
  int jaccel;
  int lwalk;
  int rwalk;
  int uphold;
  int jump;

  mobile player;

  int cam[2];
  int cto[2];
} pstate[6];

mobile players[6];



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








int shape_lookup(int X, int Y, int si, int sj, zone* z){
  int I = (X/PIXUP)/16 - si*20 - z->x;
  int J = (Y/PIXUP)/16 - sj*15 - z->y;

  int sioff = 0;
  int sjoff = 0;

  if(I>=20){
    sioff=1;
    I -= 20;
  }
  if(I<0){
    sioff=-1;
    I += 20;
  }
  if(J>=15){
    sjoff=1;
    J -= 15;
  }
  if(J<0){
    sjoff=-1;
    J += 15;
  }

//printf("%d %d %d %d\n",I,J,sioff,sjoff);


  struct screen* scr = ZONE_LOOKUP(z,si+sioff,sj+sjoff);
  return z->tileset_shapes[scr->tiles[I][J]];
}



void player_update(int id){
  struct pstate* ps = pstate+id;
  mobile* p = &(pstate[id].player);

  /*collision with stage*/

  //stage_collision(p,game.current_zone,game.si,game.sj);

  struct box* B = &(p->box);
    
  //zone* z = game.current_zone;
  //int si = game.si;
  //int sj = game.sj;

  zone* z = p->z;
  int si = p->si;
  int sj = p->sj;



  int walk = ps->rwalk - ps->lwalk;
  if(walk < 0 && p->vx > -120) p->vx += walk*2;
  else if(walk > 0 && p->vx < 120) p->vx += walk*2;
  else if(walk == 0 && ps->state == GROUND && p->vx != 0){
    if(abs(p->vx)<15){
      p->vx = 0;
    }
    else if(p->vx > 0) p->vx -= 5;
    else if(p->vx < 0) p->vx += 5;
  }





  if(p->vx > 0){
    int X = B->x + B->w + p->vx*dt;
    int shape1 = shape_lookup(X,B->y,si,sj,z);
    int shape2 = shape_lookup(X,B->y+B->h-1,si,sj,z);
    if(shape1||shape2){//tile is solid
      int P = (16*PIXUP);
      p->x = X/P*P - B->w;
      p->vx = 0;
    }
  }
  else if(p->vx < 0){
    int X = B->x + p->vx*dt;
    int shape1 = shape_lookup(X,B->y,si,sj,z);
    int shape2 = shape_lookup(X,B->y+B->h-1,si,sj,z);
    if(shape1||shape2){//tile is solid
      int P = (16*PIXUP);
      p->x = X/P*P + P;
      p->vx = 0;
    }
  }


  if(ps->state==JUMPING){//jumping
    p->vy += ps->jaccel;
    if(p->vy > 0) ps->state = FALLING;
  }
  else if(ps->state == FALLING){
    if(p->vy < 200){
      p->vy += 5;
    }
  }
  else if(ps->state == GROUND){
    p->vy += 1;
  }

  if(p->vy < 0){
    int Y = B->y + p->vy*dt;
    int X = B->x + p->vx*dt;
    int shape1 = shape_lookup(X,Y,si,sj,z);
    int shape2 = shape_lookup(X+B->w-1,Y,si,sj,z);
    if(shape1||shape2){//tile is solid
      int P = (16*PIXUP);
      p->y = Y/P*P + P;
      p->vy = 0;
    }
  }
  else if(p->vy > 0){
    int Y = B->y + B->h + p->vy*dt;
    int X = B->x + p->vx*dt;
    int shape1 = shape_lookup(X,Y,si,sj,z);
    int shape2 = shape_lookup(X+B->w-1,Y,si,sj,z);
    if(shape1||shape2){//tile is solid
      int P = (16*PIXUP);
      p->y = Y/P*P - B->h;
      p->vy = 0;
      ps->state = GROUND;
    }
  }


  /*falling off cliff*/
  if(ps->state == GROUND && p->vy > 0){
    ps->state = FALLING;
  }

  update_mobile_motion(p);

  p->si = p->x/PIXUP/16/20 - z->x;
  p->sj = p->y/PIXUP/16/15 - z->y;

  game.si = p->si;
  game.sj = p->sj;
}



static struct {
  int x, y;
  int xtar, ytar;
} cam;

void camera_update(){
  struct pstate* ps = pstate+0;
  mobile* p = &(pstate[0].player);

  if(p->vx > 0){
    cam.xtar += 800; 
    if(cam.xtar > 100*PIXUP) cam.xtar = 100*PIXUP;
  }
  else if(p->vx < 0){
    cam.xtar -= 800;
    if(cam.xtar < -100*PIXUP) cam.xtar = -100*PIXUP;
  }
  else{
    //cam.xtar -= cam.xtar/75;
  }

  cam.ytar = ps->uphold * 100 * PIXUP;

  int xdiff = cam.x - (p->x + cam.xtar);
  int ydiff = cam.y - (p->y + cam.ytar);
  
  cam.x -= xdiff/50;
  cam.y -= ydiff/50;

  point_camera(cam.x/PIXUP - 320/2, cam.y/PIXUP - 240/2);
}

void player_press(int id, int key){
  struct pstate* ps = pstate+id;
  mobile* p = &(pstate[id].player);

  switch(key){
    case LEFT_KEY:
      p->facing = LEFT;
      ps->lwalk = 1;
      break;
    case RIGHT_KEY:
      p->facing = RIGHT;
      ps->rwalk = 1;
      break;
    case JUMP_KEY:
      if(ps->state == 0){
        ps->state = JUMPING;
        p->vy = -200;
        ps->jaccel = 2;
      } 
      break;
    case UP_KEY:
      ps->uphold = -1;
      break;
    case DOWN_KEY:
      ps->uphold = 1;
      break;
  }
}

void player_release(int id, int key){
  struct pstate* ps = pstate+id;
  //mobile* p = &(pstate[id].player);

  switch(key){
    case LEFT_KEY:
      ps->lwalk = 0;
      break;
    case RIGHT_KEY:
      ps->rwalk = 0;
      break;
    case JUMP_KEY:
      ps->state = FALLING;
      break;
    case UP_KEY:
      ps->uphold = 0;
      break;
    case DOWN_KEY:
      ps->uphold = 0;
      break;
  }
}

void player_init(int id){
  struct pstate* ps = pstate+id;
  mobile* p = &(pstate[id].player);


  p->spr = enable_sprite(SPR_BOX);

  p->box.w = (p->spr->w - 2) * PIXUP;
  p->box.h = (p->spr->h - 2) * PIXUP;

  p->x = 50*PIXUP;
  p->y = 50*PIXUP;
  p->vx = 0;
  p->vy = 0;
  p->xoff = 1;
  p->yoff = 2;
  p->bxoff = 0;
  p->byoff = 0;
  update_mobile_motion(p);

  p->si = 0;
  p->sj = 0;
  p->z = game.zones[0];

  ps->state = FALLING;
  //ps->state = GROUND;
  ps->lwalk = 0;
  ps->rwalk = 0;
  ps->jaccel = 0;
  ps->uphold = 0;
}




void game_keydown(int key){
  if(key==ESCAPE_KEY){
    end_program();
  }
  player_press(0, key);
}

void game_keyup(int key){
  player_release(0, key);
}

void game_joymovex(int joy, int x){

}

void game_joymovey(int joy, int y){

}

void game_joypress(int joy, int button){

}

void game_joyrelease(int joy, int button){

}

struct handler game_handler = {
game_keydown,game_keyup,game_joymovex,
game_joymovey,game_joypress,game_joyrelease
};

void game_update(){
  player_update(0);
  camera_update();
  console_printf("%d fps",get_fps());
  console_printf("state: %s",statenames[pstate[0].state]);
}

void game_draw(){
  draw_stage();
  draw_sprites();
}

void game_setup(){
  set_handler(game_handler);
  game.update = game_update;
  game.draw = game_draw;

  game.rng = pseed(0);



  load_zone("myzone");
  game.current_zone = game.zones[0];
  enable_stage(1);

  load_sprite("box.spr",SPR_BOX);
  player_init(0);


  cam.x = 0;
  cam.y = 0;
  cam.xtar = 0;
  cam.ytar = 0;

  camera_update();
  camera_update();
  camera_update();
}
