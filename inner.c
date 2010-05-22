#include <stdio.h>
#include <stdlib.h>


//#include <root.h>

#include <list.h>
#include <util.h>

#include <input.h>
//#include <entity.h>
#include <transfer.h>
#include <gameover.h>

#include <console.h>

#include <stage.h>
#include <camera.h>
#include <hud.h>

int cx = 0;
int cy = 0;
int cvx = 0;
int cvy = 0;

static void update(){
	/* 
	update camera
	update every entity in the update list
	dispatch collision events
	execute spawn and delete events
	*/
	cx += cvx;
	cy += cvy;
	point_camera(cx, cy);
}

static void draw(){
	stage_draw_bg(cx, cy, 0, 0, 320, 240);
	//entity_draw_visible(cx, cy);
	stage_draw_fg(cx, cy, 0, 0, 320, 240);
	//hud_draw(cx, cy);
}



static void press(input in){
	if(in.button == ESCAPE_KEY){
		game_is_over();
		return;
	}
	printf("press: %s\n", input_str(in));

	switch(in.button){
		case LEFT_BUTTON: cvx += -1; break;
		case RIGHT_BUTTON: cvx += 1; break;
		case UP_BUTTON: cvy += -1; break;
		case DOWN_BUTTON: cvy += 1; break;
		default: break;
	}

	/*
	send input events
	*/
}

static void release(input in){
	printf("release: %s\n", input_str(in));

	/*
	send input events
	*/
	switch(in.button){
		case LEFT_BUTTON: cvx -= -1; break;
		case RIGHT_BUTTON: cvx -= 1; break;
		case UP_BUTTON: cvy -= -1; break;
		case DOWN_BUTTON: cvy -= 1; break;
		default: break;
	}
}

void setup_inner(){
	/* create entities */
	console_clear();
	set_handler(update, draw, press, release);


	unload_zone();
	int x = load_zone("woods");
	if(x < 0){
		error_msg("inner: cannot load zone\n");
		exit(-1);
	}
	else{
		//print_zone(x);
		
		stage_debug();
		//unload_zone();
		switch_stage("base");
	}


}







/*
stage - the stage, collision with stage, stage events
entity - moving, colliding, active/inactive stuff
inner - update draw press release



this is the inner file
the starting point to the internal game workings
independent of i/o, game loops, or graphics details

draw - draw the current state of the game
use current camera pos to...
draw the stage, efficiently draws the tiles and background
draw visible entities
draw effects
draw foreground decorations
draw gui elements

update - 
move entities / execute collision
update camera
deactivate certain entities

press -
do global actions like pause, open menu
do player specific actions

setup -


*/
