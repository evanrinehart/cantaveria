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

#include <stdio.h>

#include <video.h>
#include <input.h>
#include <kernel.h>

void main_loop(){
	int T = 0;
	while(!is_game_over()){
		int i;
		T += since();
		for(i=0; i<T/dt; i++){
			update();
		}
		if(T/dt > 0){
			draw();
			T %= dt;
		}
		delay(DELAY_AMOUNT);
	}
}

int main(int argc, char* argv[]){
	initialize(argc, argv);
	main_loop();
	return 0;
}

