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

struct stghit {
	int yes;
	int x, y;
	int vx, vy;
	int surface;
	int depth;	
};

int load_zone(string filename);
void unload_zone(void);
void stage_debug(void);

void switch_stage(string id);

void stage_draw_fg(int cx, int cy);
void stage_draw_bg(int cx, int cy);


struct stghit stage_collide(int x, int y, int vx, int vy);
struct stghit stage_collide_rect(int x, int y, int w, int h, int vx, int vy);

