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
//#include "game.h"
#include "loader.h"
#include "sound.h"

SDL_Surface* video;
int gl_flag = 0;
int fullscreen = 0;
int W, H;
int end = 0;

struct handler handler;


void set_handler(struct handler h){
	handler = h;
}

void end_program(){
	end = 1;
}

int ended(){
	return end;
}

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
int alphanum_enable = 0;

int keynum(int name){
	return keymap[name];
}

int butnum(int joy, int name){
	return butmap[joy][name];
}



int joy2player(int joy){
	int i;
	for(i=0; i<MAX_PLAYERS; i++){
		if(joymap[i] == joy) return i;
	}
	return -1;
}

int key2name(int sym){
	int i;
	for(i=0; i<14; i++){
		if(keymap[i] == sym) return i;
	}
	return -1;
}

int button2name(int player, int but){
	int i;
	if(player < 0) return -1;
	for(i=0; i<8; i++){
		if(butmap[i][player] == but) return i;
	}
	return -1;
}




void backend_quit(){
	printf("sdl: quit\n");
	SDL_LockAudio();
	SDL_CloseAudio();
	SDL_Quit();
}





void enable_alphanum(int yn){
	SDL_EnableUNICODE(yn);
	alphanum_enable = yn;
}

void input(){

	SDL_Event e;
	int name;
	int player;
	Uint16 uni16;

	while(SDL_PollEvent(&e) != 0){
		switch(e.type){
			case SDL_KEYDOWN:
				uni16 = e.key.keysym.unicode;
				if(uni16 != 0 && alphanum_enable){
					//handle uni16
				}
				else{
					name = key2name(e.key.keysym.sym);
					if(name > -1) handler.keydown(name);
				}
				break;
			case SDL_KEYUP:
				name = key2name(e.key.keysym.sym);
				if(name > -1) handler.keyup(name);
				break;
			case SDL_JOYBUTTONDOWN:
				player = joy2player(e.jbutton.which);
				name = button2name(player, e.jbutton.button);
				if(name > -1) handler.joypress(player, name);
				break;
			case SDL_JOYBUTTONUP:
				player = joy2player(e.jbutton.which);
				name = button2name(player, e.jbutton.button);
				if(name > -1) handler.joyrelease(e.jbutton.which, name);
				break;
			case SDL_JOYAXISMOTION:
				player = joy2player(e.jaxis.which);
				if(player < 0) break;
				if(e.jaxis.axis == 0){
					handler.joymovex(player, e.jaxis.value);
				}
				else if(e.jaxis.axis == 1){
					handler.joymovey(player, e.jaxis.value);
				}
				break;
			case SDL_QUIT:
				end_program();
				break;
		}
	}
}

void control(int type, int par1, int par2){

	switch(type){
		case KEYDOWN:
			handler.keydown(par1);
			break;
		case KEYUP:
			handler.keyup(par1);
			break;
		case JOYMOVEX:
			handler.joymovex(par1, par2);
			break;
		case JOYMOVEY:
			handler.joymovey(par1, par2);
			break;
		case JOYPRESS:
			handler.joypress(par1, par2);
			break;
		case JOYRELEASE:
			handler.joyrelease(par1, par2);
			break;
		default:
			report_error("backend: invalid control %d\n", type);
	}

}

struct {
	char* filename;
	SDL_Surface* surf;
	GLuint texture;
	int w;
	int h;
} gfx[MAX_GFX];
int gfx_count = 0;

int screen_w = 320;
int screen_h = 240;
int screen_offset_x = 0;
int screen_offset_y = 0;




/********************/
/* drawing routines */
/********************/

void draw_gfx_raw(int gfxid, int x, int y, int X, int Y, int W, int H){
	if(x > screen_w || y > screen_h || x+W < 0 || y+H < 0) return;
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

void draw_gfx(int gfxid, int x, int y, int X, int Y, int W, int H){
	draw_gfx_raw(gfxid, x+screen_offset_x, y+screen_offset_y, X, Y, W, H);
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
	printf("load_pixmap: %s has bpp=%d\n",filename, bpp);
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
	int i, j;

	if(gfx_count == MAX_GFX){
		fatal_error(
				"load_gfx: cannot load any more than %d graphics\n",
				MAX_GFX
			   );

	}

	printf("loading %s\n",filename);


	for(i=0; i<gfx_count; i++){/*check for already loaded gfx*/
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

		Uint32 key = SDL_MapRGB(
				surf->format,
				(COLOR_KEY&0xff0000)>>16,
				(COLOR_KEY&0x00ff00)>>8,
				(COLOR_KEY&0x0000ff)>>0
				);
		SDL_SetColorKey(surf, SDL_SRCCOLORKEY, key);

		SDL_FreeSurface(src);


		gfx[gfx_count].filename = strxcpy(filename);
		gfx[gfx_count].surf = surf;
		gfx[gfx_count].w = surf->w;
		gfx[gfx_count].h = surf->h;
	}
	else {
		GLuint texture;

		//SDL_Surface* conv = SDL_CreateRGBSurface(0, pot(src->w), pot(src->h), 32,
		//0xff<<16,0xff<<8,0xff<<0,0);

		//SDL_Surface* tmp = SDL_
		SDL_Surface* conv = SDL_DisplayFormatAlpha(src);
		//SDL_SetAlpha(conv, 0, 0);
		//SDL_BlitSurface(src, NULL, conv, NULL);

		//printf("bpp = %d\n",conv->format->BitsPerPixel);
		int N = 0;
		int M = 3;
		Uint8* conv_bytes = conv->pixels;
		Uint32 key = SDL_MapRGB(
				src->format,
				(COLOR_KEY&0xff0000)>>16,
				(COLOR_KEY&0x00ff00)>>8,
				(COLOR_KEY&0x0000ff)>>0
				);
		for(i=0; i<src->w; i++){
			for(j=0; j<src->h; j++){

				//if(1){printf("M==%d totalbytes=%d\n",M,src->w*src->h*4);}

				Uint32 pixel = *((Uint32*)(src->pixels+N));
				conv_bytes[M] =
					pixel==key ?
					SDL_ALPHA_TRANSPARENT :
					SDL_ALPHA_OPAQUE;
				N += src->format->BytesPerPixel;
				M += 4;
			}
		}

		glGenTextures( 1, &texture );
		glBindTexture( GL_TEXTURE_2D, texture );

		glTexParameteri(
				GL_TEXTURE_2D,
				GL_TEXTURE_MIN_FILTER,
				GL_NEAREST
			       );

		glTexParameteri(
				GL_TEXTURE_2D,
				GL_TEXTURE_MAG_FILTER,
				GL_NEAREST
			       );

		glTexParameteri(
				GL_TEXTURE_2D,
				GL_TEXTURE_WRAP_S,
				GL_REPEAT
			       );

		glTexParameteri(
				GL_TEXTURE_2D,
				GL_TEXTURE_WRAP_T,
				GL_REPEAT
			       );

		glTexImage2D(
				GL_TEXTURE_2D,
				0, 4,
				conv->w, conv->h,
				0,
				GL_BGRA, GL_UNSIGNED_BYTE,
				conv->pixels
			    );

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



float* lout;
float* rout;
int buffer_size;

extern void process_audio(float lout[], float rout[], int len);

void audio_callback(void *userdata, Uint8 *stream, int bytes){
	int i, j;
	Sint16* out = (Sint16*)stream;
	int buflen = bytes / 2;        /* Sint16 = 2 bytes */
	int samples = buflen / 2;      /* 2 channels */

	process_audio(lout, rout, samples);

	for(i=0, j=0; i<samples; i++){
		out[j] = (Sint16)(lout[i]*32767); j++;
		out[j] = (Sint16)(rout[i]*32767); j++;
	}

}



/******************/
/* initialization */
/******************/

void sdl_init(){
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK)==-1){
		report_error("sdl: %s\n",SDL_GetError());
		exit(-1);
	}
}




void print_version(){
	char message[] =
		"This program is distributed under the terms of the GNU General\n"
		"Public License (v2) and comes with ABSOLUTELY NO WARRANTY.\n\n"

		"Send questions, comments, and bugs to evanrinehart@gmail.com\n\n"

		"Send money to:\n"
		"1850 Claiborne St\n"
		"Mandeville, LA 70448\n"
		"United States of America\n\n"

		"Thanks! :)\n"
		;

	printf("Cantaveria (v%d.%d)\n", VERSION_MAJOR, VERSION_MINOR);
	puts("Copyright 2009 Evan Rinehart\n\n");
	puts(message);
}

void print_help(){
	printf("options:\n");
	printf("  -gl   use opengl video mode\n");
	printf("  -f    use fullscreen video mode\n");
	printf("  -h    print this help\n");
	printf("  -v    print version info\n");
}

void parse_options(int argc, char* argv[], int* fullscreen, int* gl_flag){
	int i;
	for(i=0; i<argc; i++){
		if(!strcmp(argv[i], "-gl")){
			*gl_flag = 1;
			continue;
		}
		if(!strcmp(argv[i], "-f")){
			*fullscreen = 1;
		}
		if(!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")){
			print_help();
			exit(0);
		}
		if(!strcmp(argv[i], "-v")){
			print_version();
			exit(0);
		}
	}
}

void load_keymap(){
	/*
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
	 */

	keymap[ESCAPE_KEY] = SDLK_ESCAPE;
	keymap[PAUSE_KEY] = SDLK_PAUSE;

	keymap[LEFT_KEY] = SDLK_a;
	keymap[RIGHT_KEY] = SDLK_d;
	keymap[UP_KEY] = SDLK_w;
	keymap[DOWN_KEY] = SDLK_s;

	keymap[FIRE_KEY] = SDLK_j;
	keymap[JUMP_KEY] = SDLK_k;
	keymap[INVENTORY_KEY] = SDLK_i;
	keymap[SPECIAL_KEY] = SDLK_l;

	keymap[L_KEY] = SDLK_q;
	keymap[R_KEY] = SDLK_e;
	keymap[START_KEY] = SDLK_u;
	keymap[SELECT_KEY] = SDLK_o;
}

void setup_joysticks(){
	int N = SDL_NumJoysticks();
	int i;
	printf("sdl: detected %d joysticks\n",N);
	for(i=0; i<N; i++){
		if(SDL_JoystickOpen(i)){
			printf(" joy%d: %s\n", i, SDL_JoystickName(i));
		}
		else{
			printf(
					" joy%d: %s (failed to open)\n",
					i, SDL_JoystickName(i)
			      );
		}
	}

}

void setup_video(){
	int flags = 0;
	int i;

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
			for(i=1; i<10; i++){
				if(abs(H/i - 240) < min){ min = H/i - 240; n = i; }
			}
			double new_w = ((double)W)/n;
			double new_h = ((double)H)/n;
			screen_offset_x = (new_w-320)/2;
			screen_offset_y = (new_h-240)/2;
			glOrtho(0.0f, new_w, new_h, 0.0f, -1.0f, 1.0f);
			screen_w = new_w;
			screen_h = new_h;
		}
		else{
			glOrtho(0.0f, 320, 240, 0.0f, -1.0f, 1.0f);
			screen_w = 320;
			screen_h = 240;
		}

		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity();
	}
	else{
		screen_w = 320;
		screen_h = 240;
	} 

	printf("video:\n");
	printf(" resolution: %d x %d %s\n",W,H,fullscreen?"fullscreen":"windowed");
	printf(" pixel dimensions: %d x %d\n",screen_w,screen_h);
	printf(" aspect ratio: %g\n",((double)W)/H);
	printf(" opengl: %s\n",gl_flag?"yes":"no");
	printf(" x-offset: %d\n",screen_offset_x);
	printf(" y-offset: %d\n",screen_offset_y);
	printf(" video on\n");

}

void setup_audio(){

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

	lout = xmalloc(gotten.samples*sizeof(float));
	rout = xmalloc(gotten.samples*sizeof(float));
	buffer_size = gotten.samples;

	printf(" sound on\n");
	SDL_PauseAudio(0);

}


void backend_init(int argc, char* argv[]){

	sdl_init();
	parse_options(argc, argv, &fullscreen, &gl_flag);
	load_keymap();
	setup_joysticks();
	setup_video();
	setup_audio();
	srand(RANDOM_SEED);

}



/* fps */

int fps = 0;
int update_count = 0;
int draw_count = 0;

void fps_update(){
	update_count++;
	if(update_count == 100){
		fps = draw_count * 100 / update_count;
		update_count = 0;
		draw_count = 0;
	}
}

void fps_draw(){
	draw_count++;
}

int get_fps(){
	return fps;
}

