/* graphics */

/* these are graphics routines but they rely on the backend.c for actual
drawing commands. */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

#include "util.h"
#include "backend.h"
#include "graphics.h"
#include "loader.h"


/* graphics data */
sprite* sprites[MAX_SPRITES];
int sprite_count = 0;

animation* animations[MAX_ANIMATIONS];
int anim_count = 0;

int minifont_gfx = 0;

struct {
  int x, y;
} camera;


void graphics_init(){
  minifont_gfx = load_gfx("smallfont.tga");
  for(int i=0; i<MAX_ANIMATIONS; i++){
    animations[i] = NULL;
  }
}


/* drawing */

void draw_sprite(sprite* spr){
  int x = spr->x - 0;
  int y = spr->y - 0;
  int W = spr->w;
  int H = spr->h;
  int X = spr->frame.x;
  int Y = spr->frame.y;
  int g = spr->gfxid;
  draw_gfx(g, x, y, X, Y, W, H);
}

/*
void draw_screen(zone* z, int si, int sj){
  struct screen* scr = z->screens+si+z->w*sj;
  int G = z->tileset;
  int x = si*20*16 - camera.x;
  int y = sj*15*16 - camera.y;

  for(int j=0; j < 15; j++){
    for(int i=0; i < 20; i++){
      if(x > 320 || y > 240 || x < -16 || y < -16) continue;
      int id = scr->tiles[i][j].id;
      if(id != 0){
        int X = id&7;
        int Y = id>>3;
        draw_gfx(G, x, y, X, Y, 16, 16);
      }
      else{
        //draw background
      }
      x += 16;
    }
    x -= 320;
    y += 16;
  }
}*/








void draw_small_text(char* str, int x, int y){
  for(char* c=str; *c; c++){
    int X = *c & 0x0f;
    int Y = *c >> 4;
    draw_gfx(minifont_gfx, x, y, X*4, Y*9, 3, 8);
    x+=4;
  }
}

void printf_small(int x, int y, char* format, ...){
  char str[128];
  va_list ap;
  va_start(ap, format);
  vsnprintf(str, 128, format, ap);
  va_end(ap);
  str[127]='\0';
  draw_small_text(str, x, y);
}


void draw(){
  static int N = 2;

  for(int i=0; i<sprite_count; i++){
    draw_sprite(sprites[i]);
  }

  //draw_small_text(ABC,50,50);
  printf_small(50,40,"%5s %8s","N","[0,1]");
  printf_small(50,50,"%5d %8g",N,prandr(&N,0,1));

  update_video();
  clear_video();
}




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

  //int W = gfx[g].w;
  //int H = gfx[g].h;
  int W = gfx_width(g);
  int H = gfx_height(g);
  ani->w = w;
  ani->h = h;

  for(int i=0; i < frame_count; i++){
    int l, x, y;
    loader_scanline(rd, "%d %d %d", &l, &x, &y);
    ani->frame_lens[i] = l;
    ani->frames[i].x = x;
    ani->frames[i].y = y;
    ani->frames[i].x0 = ((double)x)/W;
    ani->frames[i].y0 = ((double)y)/H;
    ani->frames[i].x1 = ((double)(x+w))/W;
    ani->frames[i].y1 = ((double)(y+h))/W;
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

