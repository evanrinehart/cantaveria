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

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <graphics.h>
#include <console.h>


char console[25][80];
int console_time[25];
int console_ptr = 0;
int console_timer = 0;

void console_clear(){
	console_ptr = 0;
}

void console_scroll(int n){
	int i;
	if(console_ptr - n < 0) n = console_ptr;
	if(n == 0) return;
	for(i=n; i<25; i++){
		memcpy(console[i-n], console[i], 80);
	}
	console[25-n][0] = '\0';
	console_ptr -= n;
}

void console_update(){
	if(console_timer == 0) return;
	console_timer -= 1;
	if(console_timer == 0 && console_ptr > 0){
		console_scroll(1);
		console_timer = 10;
	}
}

void console_printf(char* format, ...){
	if(console_ptr==25){
		console_scroll(1);
	}
	va_list ap;
	va_start(ap, format);
	vsnprintf(console[console_ptr], 80, format, ap);
	console[console_ptr][79] = '\0';
	console_ptr++;
	console_timer = 200;
	va_end(ap);
}

void console_draw(){
	int i;
	for(i=0; i<console_ptr; i++){
		printf_small(1,9*i+1,"%s",console[i]);
	}
}

