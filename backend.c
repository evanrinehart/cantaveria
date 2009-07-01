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
#include <SDL/SDL_opengl.h>

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

sprite* sprites[MAX_SPRITES];
int sprite_count = 0;

animation* animations[MAX_ANIMATIONS];
int anim_count = 0;

int stage_enabled = 0;

int screen_offset_x = 0;
int screen_offset_y = 0;

struct {
  int x, y;
} camera;



/********************/
/* drawing routines */
/********************/

void draw_sprite_sdl(sprite* spr){
  int x = spr->x - 0;
  int y = spr->y - 0;
  int w = spr->w;
  int h = spr->h;

  int X = spr->frame.x;
  int Y = spr->frame.y;

  SDL_Surface* surf = gfx[spr->gfxid].surf;
  SDL_Rect r1 = {X,Y,w,h};
  SDL_Rect r2 = {x,y,w,h};
  SDL_BlitSurface(surf,&r1,video,&r2);
}

void draw_sprite_gl(sprite* spr){
  int x = spr->x - camera.x + screen_offset_x;
  int y = spr->y - camera.y + screen_offset_y;
  int w = spr->w;
  int h = spr->h;

  glBindTexture( GL_TEXTURE_2D, gfx[spr->gfxid].texture );

  double X0 = spr->frame.x0;
  double Y0 = spr->frame.y0;
  double X1 = spr->frame.x1;
  double Y1 = spr->frame.y1;

  glBegin( GL_QUADS );
    glTexCoord2d(X0,Y0);
    glVertex3f(x,y,0);

    glTexCoord2d(X1,Y0);
    glVertex3f(x+w,y,0);

    glTexCoord2d(X1,Y1);
    glVertex3f(x+w,y+h,0);

    glTexCoord2d(X0,Y1);
    glVertex3f(x,y+h,0);
  glEnd();
}


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

    //draw_gfx_gl(gfxid, x, y, X0, X1, Y0, Y1);
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



void draw_screen_sdl(zone* z, int si, int sj){
  struct screen* scr = z->screens+si+z->w*sj;
  SDL_Surface* surf = gfx[z->tileset].surf;
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




void draw_screen_gl(zone* z, int si, int sj){


}



void draw(){

  char ABC[64] = "hello world! 45% 3.567";

  if(!gl_flag){

    SDL_FillRect(video, 0, 0);

    //draw walls and background
    if(stage_enabled){
      draw_screen_sdl(zones[0],0,0);
    }

    //draw sprites
    for(int i=0; i < sprite_count; i++){
      draw_sprite_sdl(sprites[i]);
    }

    //draw foreground tile/sprites


    draw_small_text(ABC,50,50);


    SDL_UpdateRect(video,0,0,0,0);
  }
  else{

    glClearColor(0.0,0.0,0.0,0.0);
    glClear( GL_COLOR_BUFFER_BIT );

    if(stage_enabled){
      draw_screen_gl(zones[0],0,0);
    }

    for(int i=0; i < sprite_count; i++){
      draw_sprite_gl(sprites[i]);
    }

    draw_small_text(ABC,50,50);


    SDL_GL_SwapBuffers();
  }

}


/*******************/
/* message control */
/*******************/

typedef struct {
  utf32 u;
  int gfx;
  int x, y, w, h;
  int k1, k2;
} vwchar;

struct treenode* chartree = NULL;

int minifont_gfx = 0;


int ptrcomp(void* k1, void* k2){
  return (k2 - k1);
}

void set_message(char* str){
  utf32 u;
  int N = unicode_getc(str, &u);
  while(u) {
    vwchar* C = tree_search(chartree, ptrcomp, (void*)u);
    if(C){
/* append this character to the message
   if the current word is too long,
   move the current word to the next line
   if the current word is longer than a whole line
   then just break here (would happen with japanese).
 */
      printf("%04lx[%lc] ",u, C->u);
    }
    else{
/*
character not found, so use a rectangle or something
use four tiny numbers to indicate the character
do the same as above
*/
      printf("%04lx[???] ", u);
    }
    N += unicode_getc(str+N, &u);
  };
  printf("\n");
}

void advance_message(){

}

void clear_message(){

}

void complete_message(){

}


void init_text(){
  vwchar* C = xmalloc(sizeof(vwchar));
  C->gfx = 0;
  C->u = ' ';
  C->x = 0;
  C->y = 0;
  C->w = 7;
  C->k1 = 0;
  C->k2 = 0;
  chartree = xmalloc(sizeof(treenode));
  chartree->l = NULL;
  chartree->r = NULL;
  chartree->value = C;
  chartree->key = (void*)' ';

  //set font height

  minifont_gfx = load_gfx("smallfont.tga");
}


vwchar* load_vwchar(reader* rd, int gfx){
  utf32 u;
  int x, y, w, k1, k2;
  char str[256];
  int ret = loader_scanline(rd, "%256s %d %d %d %d %d\n",str,&x,&y,&w,&k1,&k2);
  if(ret == EOF){
    return NULL;
  }
  unicode_getc(str,&u);
  vwchar* C = xmalloc(sizeof(vwchar));
  C->gfx = gfx;
  C->u = u;
  C->x = x;
  C->y = y;
  C->w = w;
  C->k1 = k1;
  C->k2 = k2;
  return C;
}

void print_tree(treenode* node){
  printf("(%lx,",(utf32)node->key);
  if(node->l){
    print_tree(node->l);
  }
  else{
    printf("()");
  }
printf(",");
  if(node->r){
    print_tree(node->r);
  }
  else{
    printf("()");
  }
  printf(")");
}


void randomly_insert(vwchar* C[], int count){
  for(int i=0; i<count-1; i++){
    int j = randint(0,count-i-1);
    tree_insert(chartree, ptrcomp, (void*)C[j]->u, C[j]);
    C[j] = C[count-i-1];
    C[count-i-1] = NULL;
  }
}


int load_font(char* filename){
  printf("load_font: loading %s\n",filename);
  char buf[256] = "fonts/";
  strmcat(buf, filename, 256);
  reader* rd = loader_open(buf);
  if(!rd){
    fatal_error("load_font: cannot open %s\n",filename);
  }

  char str[256];
  loader_scanline(rd, "%256s", str);
  int gfx = load_gfx(str);

  /* we read 64 characters at a time and insert them
     randomly into the binary search tree. this is supposed
     to help produce a more balanced tree. */
  vwchar* C[64];
  int ptr = 0;

  C[ptr] = load_vwchar(rd, gfx);
  while(C[ptr]){
    if(ptr==64){
      randomly_insert(C, 64);
      ptr = 0;
    }
    else{
      C[++ptr] = load_vwchar(rd, gfx);
    }
  }

  randomly_insert(C, ptr);

printf("load_font: character tree is the following\n");
print_tree(chartree);
printf("\n");

  return 0;
}



void draw_small_text(char* str, int x, int y){
  for(char* c=str; *c; c++){
    int X = *c & 0x0f;
    int Y = *c >> 4;
    draw_gfx(minifont_gfx, x, y, X*4, Y*9, 3, 8);
    x+=4;
  }
}

/***********/
/* utility */
/***********/

void point_camera(int x, int y){
  camera.x = x;
  camera.y = y;
}

void animate_sprites(){
  for(int i=0; i<sprite_count; i++){
    sprite* spr = sprites[i];

    spr->frame_counter += dt;
    animation* ani = animations[spr->anim];


    while(spr->frame_counter > ani->frame_lens[spr->current_frame]){
      spr->frame_counter -= ani->frame_lens[spr->current_frame];
      spr->current_frame++;
      if(spr->current_frame == ani->frame_count){
        spr->current_frame = 0;
      }
      spr->frame = ani->frames[spr->current_frame];
    }

    if(spr->update) spr->update(spr, spr->userdata);

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

int load_sprite(char* filename, int id){
  printf("loading %s\n",filename);

  char path[1024] = "sprites/";
  strncat(path, filename, 1023 - strlen(filename));

  reader* rd = loader_open(path);
  if(!rd){
    return -1;
  }

  animation* ani = xmalloc(sizeof(animation));

  char str[256];
  int w, h;
  int frame_count;
  int loop_mode;
  
  loader_scanline(rd,"%256s",str);
  loader_scanline(rd,"%d %d %d %d",&w,&h,&loop_mode,&frame_count);

  ani->frame_lens = xmalloc(frame_count*sizeof(short));
  ani->frames = xmalloc(frame_count*sizeof(struct frame));
  ani->frame_count = frame_count;

  int g = load_gfx(str);
  if(g < 0)
    return -1;

  ani->gfxid = g;

  int W = gfx[g].w;
  int H = gfx[g].h;
  ani->w = w;
  ani->h = h;

  for(int i=0; i < frame_count; i++){
    int l, x, y;
    loader_scanline(rd, "%d %d %d", &l, &x, &y);
    ani->frame_lens[i] = l;
    if(!gl_flag){
      ani->frames[i].x = x;
      ani->frames[i].y = y;
    }
    else{
      ani->frames[i].x0 = ((double)x)/W;
      ani->frames[i].y0 = ((double)y)/H;
      ani->frames[i].x1 = ((double)(x+w))/W;
      ani->frames[i].y1 = ((double)(y+h))/W;
    }
  }

  loader_close(rd);
  animations[id] = ani;  
  return 0;
}




/********************/
/* graphics control */
/********************/

sprite* enable_sprite(int sprnum){
  if(!animations[sprnum]){
    fatal_error("enable_sprite: you just tried to enable sprite with type %d, which does not exist\n",sprnum);
  }
  if(sprite_count == MAX_SPRITES){
    /* need a priority based way to create important sprites if full */
    return NULL;
  }
  sprite* spr = xmalloc(sizeof(sprite));
  animation* ani = animations[sprnum];

  spr->number = sprite_count;
  spr->frame_counter = 0;
  spr->current_frame = 0;
  spr->frame = ani->frames[0];
  spr->gfxid = ani->gfxid;
  spr->anim = sprnum;
  spr->x = 0;
  spr->y = 0;
  spr->w = ani->w;
  spr->h = ani->h;
  spr->vx = 0;
  spr->vy = 0;
  spr->update = NULL;
  spr->userdata = NULL;

  sprites[sprite_count++] = spr;
  return spr;
}

void disable_sprite(sprite* spr){
  sprite* tmp = sprites[spr->number];
  sprites[spr->number] = sprites[sprite_count--];
  free(tmp);
}

sprite* copy_sprite(sprite* spr){
  if(sprite_count == MAX_SPRITES){
    /* need way to make important sprites when full */
    return NULL;
  }
  sprite* copy = xmalloc(sizeof(sprite));
  *copy = *spr;
  sprites[sprite_count++] = copy;
  return copy;
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



  for(int i=0; i<MAX_ANIMATIONS; i++){
    animations[i] = NULL;
  }

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
  double aspect = ((double)W) / H;


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



  init_text();


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
