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
*/

#include <stdio.h>

#include <SDL/SDL.h>

#include <root.h>     /* QUANTUM SLEEP_MS */
#include <video.h>    /* since() delay() */
#include <kernel.h>   /* initialize() draw() update() */
#include <gameover.h> /* is_game_over() */

void main_loop(){
	int t = 0;
	while(!is_game_over()){
		int i;
		t += since();
		for(i=0; i < t/QUANTUM; i++){
			update();
		}
		if(t/QUANTUM > 0){
			draw();
			t %= QUANTUM;
		}
		delay(SLEEP_MS);
	}
}

int main(int argc, char* argv[]){
	initialize(argc, argv);
	main_loop();
	return 0;
}

