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


mobile player;


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
  int jump;
  mobile player;

  int cam[2];
  int cto[2];
} pstate[6];

mobile players[6];

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

int cx = 0;
int cy = 0;

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
    if(abs(p->vx)<5) p->vx = 0;
    else if(p->vx > 0) p->vx -= 3;
    else if(p->vx < 0) p->vx += 3;
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

  //point_camera(p->spr->x-320/2, p->spr->y-240/2);

  ps->cto[0] = (p->facing==LEFT ? -100 : 100)*PIXUP;
  ps->cto[1] = 0;

  cx = (cx*29 + (p->x + 320/4*PIXUP*(p->facing==LEFT?-1:1)))/30;
  cy = (cy*29 + (p->y + 240/4*PIXUP*0))/30;

  int xdiff = ps->cam[0] - (p->x + ps->cto[0]);
  int ydiff = ps->cam[1] - (p->y + ps->cto[1]);
  if(abs(xdiff) > 20*PIXUP){
  if(xdiff > 0){
    ps->cam[0] -= 1000;
  }
  if(xdiff < 0){
    ps->cam[0] += 1000;
  }
  }

  if(abs(ydiff) > 20*PIXUP){
  if(ydiff > 0){
    ps->cam[1] -= 1000;
  }
  if(ydiff < 0){
    ps->cam[1] += 1000;
  }
  }
  point_camera(ps->cam[0]/PIXUP - 320/2, ps->cam[1]/PIXUP - 240/2);

  p->si = p->x/PIXUP/16/20 - z->x;
  p->sj = p->y/PIXUP/16/15 - z->y;

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
  }
}

void player_init(int id){
  struct pstate* ps = pstate+id;
  mobile* p = &(pstate[id].player);


  p->spr = enable_sprite(SPR_BOX);

  p->box.w = (p->spr->w) * PIXUP;
  p->box.h = (p->spr->h - 2) * PIXUP;

  p->x = 50*PIXUP;
  p->y = 50*PIXUP;
  p->vx = 0;
  p->vy = 0;
  p->xoff = 0;
  p->yoff = 2;
  p->bxoff = 0;
  p->byoff = 0;
  update_mobile_motion(p);

  p->si = 0;
  p->sj = 0;
  p->z = game.zones[0];

  //ps->state = FALLING;
  ps->state = GROUND;
  ps->lwalk = 0;
  ps->rwalk = 0;
  ps->jaccel = 0;
}
  

void player_show(int id){
}

void player_hide(int id){

}


double t;

void intro_setup(){
  set_handler(intro_handler);
  game.update = intro_update;
  game.draw = intro_draw;

  load_zone("myzone");
  game.current_zone = game.zones[0];
  enable_stage(1);

  load_sprite("box.spr",SPR_BOX);
  player_init(0);

}

void intro_update(){
  player_update(0);
  console_printf("%d fps",get_fps());
  console_printf("state: %s",statenames[pstate[0].state]);
}

void intro_draw(){
  draw_stage();
  draw_sprites();
}



void intro_keydown(int key){

  player_press(0, key);

  if(key == ESCAPE_KEY){
    end_program();
  }
}

void intro_keyup(int key){
  player_release(0, key);
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

struct handler intro_handler = {
intro_keydown,intro_keyup,intro_joymovex,
intro_joymovey,intro_joypress,intro_joyrelease
};


