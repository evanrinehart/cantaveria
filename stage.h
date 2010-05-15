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


int load_zone(char* filename);
void unload_zone();
void stage_debug();

void switch_stage(char* id);

void stage_draw_fg(int cx, int cy, int x, int y, int w, int h);
void stage_draw_bg(int cx, int cy, int x, int y, int w, int h);

int stage_xcollide(int x, int y, int w, int h, int v, int* xx);
int stage_ycollide(int x, int y, int w, int h, int v, int* yy);

