/*
   Cantaveria - action adventure platform game
   Copyright (C) 2009 2010 Evan Rinehart

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

   evanrinehart@gmail.com
*/
#include <SDL/SDL.h>


#include <util.h>
#include <input.h>

#define JOY_MAX 32767
#define JOY_MIN -32768
#define JOY_THRESH 1000

static input NoInput = {NO_INPUT, INVALID_BUTTON, 0};
static input EndOfProgram = {END_OF_PROGRAM, INVALID_BUTTON, 0};

/* controller mappings */
struct key_map {
	SDLKey sym;

	int player;
	enum input_button button;

	struct key_map* next;
};
struct key_map* key_maps = NULL;


struct axis_map {
	Uint8 joystick;
	Uint8 axis;
	Sint16 last_value;

	int player;
	enum input_button button[2];

	struct axis_map* next;
};
struct axis_map* axis_maps = NULL;

struct jbutton_map {
	Uint8 joystick;
	Uint8 jbutton;

	int player;
	enum input_button button;

	struct jbutton_map* next;
};
struct jbutton_map* jbutton_maps = NULL;


struct key_map* kmfind(SDLKey sym){
	struct key_map* ptr = key_maps;
	while(ptr){
		if(ptr->sym == sym)
			return ptr;
		else
			ptr = ptr->next;
	}
	return NULL;
}

struct axis_map* amfind(Uint8 joystick, Uint8 axis){
	struct axis_map* ptr = axis_maps;
	while(ptr){
		if(ptr->joystick == joystick && ptr->axis == axis)
			return ptr;
		else
			ptr = ptr->next;
	}
	return NULL;
}

struct jbutton_map* jmfind(Uint8 joystick, Uint8 jbutton){
	struct jbutton_map* ptr = jbutton_maps;
	while(ptr){
		if(ptr->joystick == joystick && ptr->jbutton == jbutton)
			return ptr;
		else
			ptr = ptr->next;
	}
	return NULL;
}



enum input_type kmtype(Uint8 type){
	switch(type){
	case SDL_KEYDOWN: return BUTTON_PRESS;
	case SDL_KEYUP: return BUTTON_RELEASE;
	default:
		report_error("input map_key: invalid SDL event type\n");
		return NO_INPUT;
	}
}

enum input_type jmtype(Uint8 type){
	switch(type){
	case SDL_JOYBUTTONDOWN: return BUTTON_PRESS;
	case SDL_JOYBUTTONUP: return BUTTON_RELEASE;
	default:
		report_error("input map_jbutton: invalid SDL event type\n");
		return NO_INPUT;
	}
}

enum input_button ambutton(int state, struct axis_map* am){
	switch(state){
	case -1: return am->button[0];
	case 1: return am->button[1];
	case 0:
		report_error("input axis_motion: axis state 0 does not imply a button\n");
		return INVALID_BUTTON;
	default:
		report_error("input axis_motion: invalid axis state (%d)\n", state);
		return INVALID_BUTTON;
	}
}

input map_key(SDL_Event* e){
	SDLKey sym = e->key.keysym.sym;
	struct key_map* km = kmfind(sym);
	input in;
	if(km){
		in.type = kmtype(e->type);
		in.player = km->player;
		in.button = km->button;
		return in;
	}
	else{
		return NoInput;
	}
}

input map_jbutton(SDL_Event* e){
	Uint8 which = e->jbutton.which;
	Uint8 jbutton = e->jbutton.button;
	struct jbutton_map* jm = jmfind(which, jbutton);
	input in;

	if(jm){
		in.type = jmtype(e->type);
		in.player = jm->player;
		in.button = jm->button;
		return in;
	}
	else{
		return NoInput;
	}
}

int axis_state(Sint16 x){
	if(x < -JOY_THRESH) return -1;
	else if(x > JOY_THRESH) return 1;
	else return 0;
}

input axis_motion(Sint16 value, struct axis_map* am){
	int state0 = axis_state(am->last_value);
	int state1 = axis_state(value);
	int diff = state1 - state0;
	input in;

	if(abs(diff) == 2){
		/*stuck FIXME ?
		set a flag in the am which tells get_input
		to do a button press
		note that without this fix, it wont get stuck, 
		the input system will just be slightly inconsistent*/
		report_error("input axis_motion: (FIXME) joystick almost got stuck\n");
	}

	if(diff == 0){
		return NoInput;
	}
	else if(state0 != 0){
		in.type = BUTTON_RELEASE;
		in.player = am->player;
		in.button = ambutton(state0, am);
		return in;
	}
	else{ /*(state1 != 0)*/
		in.type = BUTTON_PRESS;
		in.player = am->player;
		in.button = ambutton(state1, am);
		return in;
	}
}

input map_jaxis(SDL_Event* e){
	Uint8 which = e->jaxis.which;
	Uint8 axis = e->jaxis.axis;
	Sint16 value = e->jaxis.value;
	struct axis_map* am = amfind(which, axis);
	if(am){
		return axis_motion(value, am);
	}
	else{
		return NoInput;
	}
}



void remap_input(enum input_button button, int player){
	/*remap_flag = 1*/
}




/* *** */
input get_input(){
	SDL_Event e;

	if(SDL_PollEvent(&e) == 0){
		return NoInput;
	}

	/*
	if(remap_flag){
		remap(&e);
	}
	*/

	switch(e.type){
	case SDL_KEYDOWN:
	case SDL_KEYUP:
		return map_key(&e);

	case SDL_JOYBUTTONDOWN:
	case SDL_JOYBUTTONUP:
		return map_jbutton(&e);

	case SDL_JOYAXISMOTION:
		return map_jaxis(&e);

	case SDL_QUIT:
		return EndOfProgram;

	default:
		return NoInput;
	}
}




void kmadd(int player, enum input_button button, SDLKey sym){
	struct key_map* km = malloc(sizeof(struct key_map));
	struct key_map* ptr = key_maps;;
	km->player = player;
	km->button = button;
	km->sym = sym;
	km->next = NULL;

	if(!ptr){
		key_maps = km;
		return;
	}

	while(ptr->next) ptr = ptr->next;

	ptr->next = km;
}

void input_init(const char* filename){
	/* set up mappings */
	// A -> Left
	// D -> Right
	// W -> Up
	kmadd(0, LEFT_BUTTON, SDLK_a);
	kmadd(0, RIGHT_BUTTON, SDLK_d);
	kmadd(0, UP_BUTTON, SDLK_w);
	kmadd(0, DOWN_BUTTON, SDLK_s);

	kmadd(0, JUMP_BUTTON, SDLK_k);
	kmadd(0, FIRE_BUTTON, SDLK_j);

	kmadd(0, ESCAPE_KEY, SDLK_ESCAPE);
	kmadd(0, PAUSE_KEY, SDLK_PAUSE);
}

void save_input(const char* filename){

}



