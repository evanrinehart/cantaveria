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

#include <SDL/SDL.h>

#include "game.h"
#include "backend.h"
#include "util.h"
#include "loader.h"

SDL_Surface* video;
int gl_flag = 0;
int W, H;

int since(){
  static int last = 0;
  int now = SDL_GetTicks();
  int diff = now - last;
  last = now;
  return diff;
}

void delay(int ms){
  SDL_Delay(ms);
}


int keymap[32];
int butmap[MAX_PLAYERS][8];
int joymap[MAX_PLAYERS];

int keynum(int name){
  return keymap[name];
}

int butnum(int joy, int name){
  return butmap[joy][name];
}


void backend_quit(){
  printf("sdl: quit\n");
  SDL_Quit();
}







void input(){

  SDL_Event e;

  while(SDL_PollEvent(&e) != 0){
    switch(e.type){
      case SDL_KEYDOWN:
        game.handler.keydown(e.key.keysym.sym);
        break;
      case SDL_KEYUP:
        game.handler.keyup(e.key.keysym.sym);
        break;
      case SDL_JOYBUTTONDOWN:
        game.handler.joypress(e.jbutton.which, e.jbutton.button);
        break;
      case SDL_JOYBUTTONUP:
        game.handler.joyrelease(e.jbutton.which, e.jbutton.button);
        break;
      case SDL_JOYAXISMOTION:
        if(e.jaxis.axis == 0){
          game.handler.joymovex(e.jaxis.which, e.jaxis.value);
        }
        else if(e.jaxis.axis == 1){
          game.handler.joymovey(e.jaxis.which, e.jaxis.value);
        }
        break;
      case SDL_QUIT:
        game.end = 1;
        break;
    }
  }
}

void control(int type, int par1, int par2){

  switch(type){
    case KEYDOWN:
      game.handler.keydown(par1);
      break;
    case KEYUP:
      game.handler.keyup(par1);
      break;
    case JOYMOVEX:
      game.handler.joymovex(par1, par2);
      break;
    case JOYMOVEY:
      game.handler.joymovey(par1, par2);
      break;
    case JOYPRESS:
      game.handler.joypress(par1, par2);
      break;
    case JOYRELEASE:
      game.handler.joyrelease(par1, par2);
      break;
  }

}












/*
drawing model

the backend will draw the current state of the game
in the following way.

it will render a stage at a given position. a stage is
N backgrounds
1 background tiles
1 wall tiles and
1 foreground tiles

then it will render the sprites


the stage is measured in terms of screens
each screen is 20x15 tiles, each tile is 16 pixels square
each screen can have one tile set of 256 tiles
*/

/*graphics data*/
SDL_Surface* gfx[256];
int gfx_count = 0;

sprite* sprites[256];
int sprite_count = 0;

sprite* animations[256];
int anim_count = 0;

int stage_enabled = 0;

struct {
  int x, y;
} camera;
/*end graphics data*/


void draw_sprite_sdl(sprite* spr){
  int x = spr->x - 0;
  int y = spr->y - 0;
  int w = spr->w;
  int h = spr->h;
  int f = spr->current_frame;

  SDL_Surface* surf = gfx[spr->gfx];
  SDL_Rect r1 = {f*w,0,w,h};
  SDL_Rect r2 = {  x,y,w,h};
  SDL_BlitSurface(surf,&r1,video,&r2);
}

void draw_sprite_gl(sprite* spr){
  int x = spr->x - camera.x;
  int y = spr->y - camera.y;
  int w = spr->w;
  int h = spr->h;
  int f = spr->frame_counter;

  //choose texture based on spr->sheet_number
  //set texture coordinates based on f*w
  //draw a quad at x y w h
  
}

void draw_screen_sdl(zone* z, int si, int sj){
  struct screen* scr = z->screens+si+z->w*sj;
  SDL_Surface* surf = gfx[z->tileset];
  int x = si*20*16 - camera.x;
  int y = sj*15*16 - camera.y;

  for(int j=0; j < 15; j++){
    for(int i=0; i < 20; i++){
      if(x > 320 || y > 240 || x < -16 || y < -16) continue;
      int gfx = scr->tiles[i][j].gfx;
      if(gfx != 0){
        SDL_Rect r1 = {gfx&7,gfx>>3,16,16};
        SDL_Rect r2 = {    x,     y,16,16};
        SDL_BlitSurface(surf,&r1,video,&r2);
      }
      else{
        //draw background
      }
      x += 16;
    }
    x -= 320;
    y += 16;
  }
}
int color = 0x0000ff00;
void draw(){

  if(!gl_flag){

    //draw walls and background
    if(stage_enabled){
      draw_screen_sdl(zones[0],0,0);
    }

    //draw sprites
    for(int i=0; i < sprite_count; i++){
      draw_sprite_sdl(sprites[i]);
    }

    //draw foreground tile/sprites

    SDL_UpdateRect(video,0,0,0,0);
  }
  else{

  }

}

void point_camera(int x, int y){
  camera.x = x;
  camera.y = y;
}



void animate_sprites(){
  for(int i=0; i<sprite_count; i++){
    sprite* spr = sprites[i];
    spr->frame_counter += dt;
    while(spr->frame_counter > spr->frame_len[spr->current_frame]){
      spr->frame_counter -= spr->frame_len[spr->current_frame];
      spr->current_frame++;
      if(spr->current_frame == spr->frame_c){
        spr->current_frame = 0;
      }
    }

    if(spr->update) spr->update(spr, spr->userdata);

  }
}




/* graphics loading */

SDL_Surface* SDL_NewSurface(int w, int h){
  SDL_Surface* tmp = SDL_CreateRGBSurface(SDL_SRCCOLORKEY,w,h,32,0,0,0,0);
  SDL_Surface* surf = SDL_DisplayFormat(tmp);
  SDL_FreeSurface(tmp);

  SDL_FillRect(surf,0,0x00ffffff);

  return surf;
}

SDL_Surface* load_pixmap(char* filename){
  char path[1024] = "gfx/";
  strncat(path, filename, 1023 - strlen(filename));
  reader* rd = loader_open(path);
  if(!rd){
    return NULL;
  }

  unsigned char header[18];
  loader_read(rd, header, 18);

  int w = header[12] + (header[13]<<8);
  int h = header[14] + (header[15]<<8);
  int bpp = header[16];

  SDL_Surface* surf = SDL_CreateRGBSurface(0,w,h,bpp,
                      0x00ff0000,0x0000ff00,0x000000ff,0);
  loader_read(rd, surf->pixels, w*(bpp/8)*h);
  loader_close(rd);

  

  return surf;
}

int load_gfx(char* filename){
  if(gfx_count == 256){
    report_error("load_gfx: cannot load %s (%d already loaded)\n",filename,256);
    return -1;
  }
  printf("loading %s\n",filename);
  char path[1024] = "gfx/";
  strncat(path, filename, 1023 - strlen(filename));
  //char* buf = (char*)loader_readall(path,NULL);
  reader* rd = loader_open(path);
  //if(!buf){
  //  return -1;
  //}

  char str[1024];
  int fcount, w, h;
  //sscanf(buf,"%s %d %d %d\n",str,&fcount,&w,&h);
  loader_scanline(rd, "%s", str);
  loader_scanline(rd, "%d %d %d\n", &fcount,&w,&h);

  SDL_Surface* src = load_pixmap(str);
  if(!src){
    //free(buf);
    loader_close(rd);
    return -1;
  }
  SDL_Surface* dst = SDL_NewSurface(fcount*w,h);

  for(int i=0; i < fcount; i++){
    int x, y;
    //sscanf(buf, "%d %d", &x, &y);
    loader_scanline(rd, "%d %d", &x, &y);
    SDL_Rect r1 = {  x,y,w,h};
    SDL_Rect r2 = {i*w,0,w,h};
    SDL_BlitSurface(src, &r1, dst, &r2);
  }

  //SDL_BlitSurface(dst,0,video,0);

  SDL_FreeSurface(src);
  //free(buf);  
  loader_close(rd);

  int black = 0;
  SDL_SetColorKey(dst, SDL_SRCCOLORKEY, black);

  gfx[gfx_count++] = dst;
  return gfx_count-1;
}

int load_sprite(char* filename, int sprnum){
  printf("loading %s\n",filename);
  char path[1024] = "sprites/";
  strncat(path, filename, 1023 - strlen(filename));
  //char* buf = (char*)loader_readall(path, NULL);
  //if(!buf){
  //  return -1;
 // }
  reader* rd = loader_open(path);
  if(!rd){
    return -1;
  }

  sprite* spr = malloc(sizeof(sprite));
  if(!spr){
    //free(buf);
    loader_close(rd);
    return -1;
  }
  spr->number = sprnum;
  spr->x = 0;
  spr->y = 0;
  spr->vx = 0;
  spr->vy = 0;
  spr->frame_counter = 0;
  spr->current_frame = 0;
  spr->update = NULL;
  spr->userdata = NULL;

  char str[1024];
  int w, h;
  int loop_mode;
  int frame_c;
  
  //sscanf(buf,"%s %d %d %d %d",str,&w,&h,&loop_mode,&frame_count);
  loader_scanline(rd, "%s",str);
  loader_scanline(rd, "%d %d %d %d",&w,&h,&loop_mode,&frame_c);
  spr->w = w;
  spr->h = h;
  spr->loop_mode = loop_mode;
  spr->frame_c = frame_c;

  for(int i=0; i < frame_c; i++){
    if(i==16){
      break;
    }
    int n;
    //sscanf(buf,"%d",&n);
    loader_scanline(rd, "%d", &n);
    spr->frame_len[i] = n;
  }

  //free(buf);
  loader_close(rd);

  int gfx = load_gfx(str);
  if(gfx < 0)
    return -1;
  spr->gfx = gfx;

  animations[sprnum] = spr;
  
  return 0;
}

sprite* enable_sprite(int sprnum){
  sprite* spr = copy_sprite(animations[sprnum]);
  sprites[sprite_count++] = spr;
  return spr;
}

void disable_sprite(sprite* spr){
  //swap spr and last sprite
}

sprite* copy_sprite(sprite* spr){
  sprite* copy = malloc(sizeof(sprite));
  *copy = *spr;
  return copy;
}


void backend_init(int argc, char* argv[]){

  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK)==-1){
    report_error("sdl: %s\n",SDL_GetError());
    exit(-1);
  }


  /* options */
  int fullscreen = 0;
  for(int i=0; i<argc; i++){
    if(strncmp(argv[i], "-gl",5)==0){
      gl_flag = 1;
      continue;
    }
    if(strncmp(argv[i], "-f",5)==0){
      fullscreen = 1;
    }
  }


  /* keymap */
  keymap[ESCAPE_KEY] = SDLK_ESCAPE;
  keymap[PAUSE_KEY] = SDLK_PAUSE;

  keymap[LEFT_KEY] = SDLK_LEFT;
  keymap[RIGHT_KEY] = SDLK_RIGHT;
  keymap[UP_KEY] = SDLK_UP;
  keymap[DOWN_KEY] = SDLK_DOWN;

  keymap[FIRE_KEY] = SDLK_z;
  keymap[JUMP_KEY] = SDLK_x;
  keymap[INVENTORY_KEY] = SDLK_a;
  keymap[SPECIAL_KEY] = SDLK_s;

  keymap[L_KEY] = SDLK_q;
  keymap[R_KEY] = SDLK_w;
  keymap[START_KEY] = SDLK_RETURN;
  keymap[SELECT_KEY] = SDLK_TAB;

  /* joysticks */
  int N = SDL_NumJoysticks();
  printf("sdl: detected %d joysticks\n",N);
  for(int i=0; i<N; i++){
    if(SDL_JoystickOpen(i)){
      printf(" joy%d: %s\n",i,SDL_JoystickName(i));
    }
    else{
      printf(" joy%d: %s (failed to open)\n",i,SDL_JoystickName(i));
    }
  }


  /* video */
  int flags = 0;

  SDL_WM_SetCaption("cantaveria","cantaveria");
  SDL_ShowCursor(SDL_DISABLE);
  const SDL_VideoInfo* vinfo = SDL_GetVideoInfo();
  
  double aspect = vinfo->current_w*1.0 / vinfo->current_h;

printf("aspect ratio = %g\n",aspect);
printf("current rez = %d x %d\n",vinfo->current_w,vinfo->current_h);
  if(fullscreen && gl_flag){
    W = vinfo->current_w;
    H = vinfo->current_h;
  }
  else if(gl_flag){
    W = 800;
    H = 600;
  }
  else if(fullscreen){
    W = 320;
    H = 240;
  }
  else{
    W = 320;
    H = 240;
  }

  if(gl_flag){
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    flags |= SDL_OPENGL;
  };

  if(fullscreen){
    flags |= SDL_FULLSCREEN;
  }

  video = SDL_SetVideoMode(W,H,32,flags);
  if(video == NULL){
    report_error("sdl: %s\n",SDL_GetError());
    exit(-1);
  }

  if(gl_flag){
    //set up opengl stuff
  }


  //atexit(backend_quit);
}