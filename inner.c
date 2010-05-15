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

static void update(){
	/* 
	update camera
	update every entity in the update list
	dispatch collision events
	execute spawn and delete events
	*/
}

static void draw(){
	int cx = camera_x();
	int cy = camera_y();
	//stage_draw_bg(cx, cy);
	//entity_draw_visible(cx, cy);
	//stage_draw_fg(cx, cy);
	hud_draw(cx, cy);
}

static void press(input in){
	if(in.button == ESCAPE_KEY){
		game_is_over();
		return;
	}
	printf("press: %s\n", input_str(in));

	/*
	send input events
	*/
}

static void release(input in){
	printf("release: %s\n", input_str(in));

	/*
	send input events
	*/
}

void setup_inner(){
	/* create entities */
	console_clear();
	set_handler(update, draw, press, release);


	unload_zone();
	int x = load_zone("3ztest/");
	if(x < 0){
		error_msg("inner: cannot load zone\n");
		exit(-1);
	}
	else{
		//print_zone(x);
		stage_debug();
		unload_zone();
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
