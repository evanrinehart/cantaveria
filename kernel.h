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
#define VERSION_MAJOR 0
#define VERSION_MINOR 0

#define MAX_PLAYERS 6

#define dt 10
#define DELAY_AMOUNT 1

#define PI 3.14159265359
#define PI2 2*PI

#define RANDOM_SEED 57

void draw();
void update();
void initialize();
void set_handler(
	void (*update)(),
	void (*draw)(),
	void (*press)(input in),
	void (*release)(input in)
);

void game_is_over();
int is_game_over();
