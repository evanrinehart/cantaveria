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

#include <math.h>

#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>

#include "util.h"
#include "backend.h"
#include "game.h"
#include "loader.h"
#include "sound.h"

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
  SDL_LockAudio();
  SDL_CloseAudio();
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

/***************/
/*graphics data*/
/***************/

struct {
  char* filename;
  SDL_Surface* surf;
  GLuint texture;
  int w;
  int h;
} gfx[MAX_GFX];
int gfx_count = 0;

int screen_offset_x = 0;
int screen_offset_y = 0;




/********************/
/* drawing routines */
/********************/

void draw_gfx(int gfxid, int x, int y, int X, int Y, int W, int H){
  if(!gl_flag){
    SDL_Surface* surf = gfx[gfxid].surf;
    SDL_Rect r1 = {X,Y,W,H};
    SDL_Rect r2 = {x,y,W,H};
    SDL_BlitSurface(surf,&r1,video,&r2);
  }
  else{
    double w = gfx[gfxid].w;
    double h = gfx[gfxid].h;
    double X0 = ((double)X)/w;
    double X1 = X0 + ((double)W)/w;
    double Y0 = ((double)Y)/h;
    double Y1 = Y0 + ((double)H)/h;

    x = x + screen_offset_x;
    y = y + screen_offset_y;
    glBindTexture( GL_TEXTURE_2D, gfx[gfxid].texture );
    glBegin( GL_QUADS );
      glTexCoord2d(X0,Y0);
      glVertex3f(x,y,0);

      glTexCoord2d(X1,Y0);
      glVertex3f(x+W,y,0);

      glTexCoord2d(X1,Y1);
      glVertex3f(x+W,y+H,0);

      glTexCoord2d(X0,Y1);
      glVertex3f(x,y+H,0);
    glEnd();
  }
}

void clear_video(){
  if(!gl_flag){
    SDL_FillRect(video, 0, 0);
  }
  else{
    glClearColor(0.0,0.0,0.0,0.0);
    glClear( GL_COLOR_BUFFER_BIT );
  }
}

void update_video(){
  if(!gl_flag){
    SDL_UpdateRect(video,0,0,0,0);
  }
  else{
    SDL_GL_SwapBuffers();
  }
}


/********************/
/* graphics loading */
/********************/

SDL_Surface* SDL_NewSurface(int w, int h){
  char prefix[32] = "SDL_NewSurface";
  SDL_Surface* tmp = SDL_CreateRGBSurface(SDL_SRCCOLORKEY,w,h,32,0,0,0,0);
  if(!tmp){
    out_of_memory(prefix);
  }

  SDL_Surface* surf = SDL_DisplayFormat(tmp);
  if(!surf){
    out_of_memory(prefix);
  }

  SDL_FreeSurface(tmp);

  SDL_FillRect(surf,0,0x00ffffff);

  return surf;
}

/*
load_pixmap filename
  loads a pixmap file stored in the gfx/ subdir
  returns an SDL surface in the 24bit format RGBRGBRGB...

load_gfx filename
  returns the gfx id of the graphics with filename
  may or may not load a new gfx object with load_pixmap
  in sdl mode this sets up the gfx surface for color key transparency
  in opengl mode this converts the 24bit to 32bit texture with alpha channel

load_sprite filename
  reads a sprite definition from from the sprites/ subdir
  loads a new animation structure as defined the file
  uses load_gfx to get gfx id for animation graphics
  therefore, may or may not load new graphics
  returns an sprite id number used to instantiate new sprites
*/



SDL_Surface* load_pixmap(char* filename){
  char path[256] = "gfx/";
  strmcat(path, filename, 256);

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
                      0x00ff0000,0x0000ff00,0x000000ff,0xff000000);
  if(!surf){
    out_of_memory("load_pixmap");
  }
  loader_read(rd, surf->pixels, w*(bpp/8)*h);
  loader_close(rd);

  return surf;
}

int load_gfx(char* filename){
  if(gfx_count == MAX_GFX){
    fatal_error("load_gfx: cannot load any more than %d graphics\n",MAX_GFX);
  }

  printf("loading %s\n",filename);


  for(int i=0; i<gfx_count; i++){/*check for already loaded gfx*/
    if(strcmp(gfx[i].filename, filename)==0){
      return i;
    }
  }

  SDL_Surface* src = load_pixmap(filename);
  if(!src){
    fatal_error("load_gfx: failed to load %s\n",filename);
  }

  if(!gl_flag){
    SDL_Surface* surf = SDL_DisplayFormatAlpha(src);
    SDL_SetAlpha(surf, 0, 0);

    Uint32 key = SDL_MapRGB(surf->format, (COLOR_KEY&0xff0000)>>16, 
                                          (COLOR_KEY&0x00ff00)>>8,
                                          (COLOR_KEY&0x0000ff)>>0);
    SDL_SetColorKey(surf, SDL_SRCCOLORKEY, key);

    SDL_FreeSurface(src);
    

    gfx[gfx_count].filename = strxcpy(filename);
    gfx[gfx_count].surf = surf;
    gfx[gfx_count].w = surf->w;
    gfx[gfx_count].h = surf->h;
  }
  else {
    GLuint texture;
    
    SDL_Surface* conv = SDL_CreateRGBSurface(0, src->w, src->h, 32,
      0xff<<16,0xff<<8,0xff<<0,0);
    
    SDL_BlitSurface(src, NULL, conv, NULL);

    int N = 0;
    int M = 3;
    Uint8* conv_bytes = conv->pixels;
    Uint32 key = SDL_MapRGB(src->format,(COLOR_KEY&0xff0000)>>16, 
                                        (COLOR_KEY&0x00ff00)>>8,
                                        (COLOR_KEY&0x0000ff)>>0);
    for(int i=0; i<src->w; i++){
      for(int j=0; j<src->h; j++){
        Uint32 pixel = *((Uint32*)(src->pixels+N));
        conv_bytes[M] = pixel==key ? SDL_ALPHA_TRANSPARENT : SDL_ALPHA_OPAQUE;
        N += src->format->BytesPerPixel;
        M += 4;
      }
    }

    glGenTextures( 1, &texture );
    glBindTexture( GL_TEXTURE_2D, texture );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D( GL_TEXTURE_2D, 0, 4, conv->w, conv->h, 0,
                  GL_BGRA, GL_UNSIGNED_BYTE, conv->pixels );

    gfx[gfx_count].filename = strxcpy(filename);
    gfx[gfx_count].texture = texture;
    gfx[gfx_count].w = src->w;
    gfx[gfx_count].h = src->h;

    SDL_FreeSurface(conv);
    SDL_FreeSurface(src);

  }

  return gfx_count++;
}

int gfx_width(int gfxid){
  return gfx[gfxid].w;
}

int gfx_height(int gfxid){
  return gfx[gfxid].h;
}



Sint16* lout;
Sint16* rout;
int buffer_size;

extern void process_audio(short lout[], short rout[], int len);

void audio_callback(void *userdata, Uint8 *stream, int len){
  Sint16* out = (Sint16*)stream;

  process_audio(lout, rout, buffer_size);

  for(int i=0; i<buffer_size; i++){
    out[i*2    ] = lout[i];
    out[i*2 + 1] = rout[i];
  }

}



/******************/
/* initialization */
/******************/

void backend_init(int argc, char* argv[]){

  srand(RANDOM_SEED);



  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK)==-1){
    report_error("sdl: %s\n",SDL_GetError());
    exit(-1);
  }


  /* options */
  int fullscreen = 0;
  for(int i=0; i<argc; i++){
    if(!strcmp(argv[i], "-gl")){
      gl_flag = 1;
      continue;
    }
    if(!strcmp(argv[i], "-f")){
      fullscreen = 1;
    }
    if(!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")){
      printf("options:\n");
      printf("  -gl   use opengl video mode\n");
      printf("  -f    use fullscreen video mode\n");
      printf("  -h    print this help\n");
      printf("  -v    print version info\n");
      exit(0);
    }
    if(!strcmp(argv[i], "-v")){
      printf("cantaveria (v%d.%d)\n",VERSION_MAJOR,VERSION_MINOR);
      printf("Copyright 2009 Evan Rinehart\n\n");

      printf("This program is distributed under the terms of the GNU General\n"
             "Public License (v2) and comes with ABSOLUTELY NO WARRANTY.\n\n");

      printf("Send questions, comments, and bugs to evanrinehart@gmail.com\n\n");

      printf("Send money to:\n");
      printf("1850 Claiborne St\n");
      printf("Mandeville, LA 70448\n");
      printf("United States of America\n\n");

      printf("Thanks! :)\n");
      exit(0);
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
  

  if(fullscreen && gl_flag){
    W = vinfo->current_w;
    H = vinfo->current_h;
  }
  else if(gl_flag){
    //W = 320;
    //H = 240;
    W = 640;
    H = 480;
    //W = 960;
    //H = 720;
  }
  else if(fullscreen){
    W = 320;
    H = 240;
  }
  else{
    W = 320;
    H = 240;
  }

  //double aspect = vinfo->current_w*1.0 / vinfo->current_h;
  //double aspect = ((double)W) / H;


  if(gl_flag){
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    //SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable( GL_TEXTURE_2D );
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );

    glViewport( 0, 0, W, H );

    glClear( GL_COLOR_BUFFER_BIT );

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    if(fullscreen){
      //glOrtho(0.0f, 240*aspect, 240, 0.0f, -1.0f, 1.0f);
      //glOrtho(0.0f, 1280.0/3, 800.0/3, 0.0f, -1.0f, 1.0f);
      int min = 9999;
      int n = 0;
      for(int i=1; i<10; i++){
        if(abs(H/i - 240) < min){ min = H/i - 240; n = i; }
      }
      double new_w = ((double)W)/n;
      double new_h = ((double)H)/n;
      screen_offset_x = (new_w-320)/2;
      screen_offset_y = (new_h-240)/2;
      glOrtho(0.0f, new_w, new_h, 0.0f, -1.0f, 1.0f);
    }
    else{
      glOrtho(0.0f, 320, 240, 0.0f, -1.0f, 1.0f);
    }

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
  }

  printf("video:\n");
  printf(" resolution: %d x %d %s\n",W,H,fullscreen?"fullscreen":"windowed");
  printf(" aspect ratio: %g\n",((double)W)/H);
  printf(" opengl: %s\n",gl_flag?"yes":"no");
  printf(" x-offset: %d\n",screen_offset_x);
  printf(" y-offset: %d\n",screen_offset_y);
  printf(" video on\n");

  /* setup audio */
  SDL_AudioSpec audio;
  audio.freq = SAMPLE_RATE;
  audio.format = AUDIO_S16;
  audio.channels = 2;
  audio.samples = BUFFER_SIZE;
  audio.callback = audio_callback;

  SDL_AudioSpec gotten;

  if(SDL_OpenAudio(&audio, &gotten)<0){
    report_error("sdl: cannot open audio (%s)\n", SDL_GetError());
    exit(-1);
  }

  printf("audio:\n");
  printf(" sample rate = %d\n",gotten.freq);
  printf(" channels = %d\n",gotten.channels);
  printf(" samples = %d\n",gotten.samples);
  printf(" format = %d\n",gotten.format);

  if(gotten.format != AUDIO_S16){
    printf("    WARNING: audio format not AUDIO_S16 :(\n");
  }

  lout = xmalloc(gotten.samples*2);
  rout = xmalloc(gotten.samples*2);
  buffer_size = gotten.samples;

  printf(" sound on\n");
  SDL_PauseAudio(0);



  /* were done here */
/*
SDL_Rect** modes;
int i;

modes = SDL_ListModes(NULL, SDL_FULLSCREEN|SDL_HWSURFACE);

if (modes == (SDL_Rect**)0) {
    printf("No modes available!\n");
    exit(-1);
}

if (modes == (SDL_Rect**)-1) {
    printf("All resolutions available.\n");
}
else{
    printf("Available Modes\n");
    for (i=0; modes[i]; ++i)
      printf("  %d x %d\n", modes[i]->w, modes[i]->h);
}
*/

}
