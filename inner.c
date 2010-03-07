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

//int run(Y* y, int dt){
/*
	motion move = get(y->pc);
	int passed = move(y->m, dt);
	if(passed < dt){
		advance(y);
	return passed;
*/
//return dt;
//}
/*
void move(Y* y){
	int t = QUANTUM;
	while(t > 0){
		int dt = QUANTUM;
		dt = run(y, dt);
		t -= dt;
		//collision here
	}
}
*/
/*
void show(Y* y){
	y->draw(y);
}
*/
static void update(){
/*	list* ents = get_active_entities();
	foreach(ents, move);*/
}

static void draw(){
	//int x = camera_x();
	//int y = camera_y();
	//int w = screen_w();
	//int h = screen_h();
/*	list* ents = find_entities_in(x,y,w,h);
	foreach(ents, show);*/
}

static void press(input in){
	if(in.button == ESCAPE_KEY){
		game_is_over();
		return;
	}
	printf("press: %s\n", input_str(in));
	/*
check global keys

do player specific side effects
	*/
}

static void release(input in){
	printf("release: %s\n", input_str(in));
}

void setup_inner(){
	// create entities
	console_clear();
	set_handler(update, draw, press, release);


	int x = load_zone("myzone");
	if(x < 0){
		error_msg("inner: cannot load zone\n");
		exit(-1);
	}
	else{
		print_zone(x);
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
