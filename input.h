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
enum input_type {
	BUTTON_PRESS,
	BUTTON_RELEASE,
	NO_INPUT,
	SKIP_INPUT,
	INVALID_INPUT,
	END_OF_PROGRAM
};

enum input_button {
	START_BUTTON,
	SELECT_BUTTON,
	L_BUTTON,
	R_BUTTON,

	LEFT_BUTTON,
	RIGHT_BUTTON,
	UP_BUTTON,
	DOWN_BUTTON,

	FIRE_BUTTON,
	JUMP_BUTTON,
	INVENTORY_BUTTON,
	SPECIAL_BUTTON,

	ESCAPE_KEY,
	PAUSE_KEY,

	INVALID_BUTTON,
	NONDESCRIPT_BUTTON,
};

typedef struct {
	enum input_type type;
	enum input_button button;
	int player;
} input;


void input_init(const char* filename);
void save_input(const char* filename);
input get_input();
void remap_input(enum input_button, int player);
const char* input_str(input in);
