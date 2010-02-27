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
#include <stdlib.h>
#include <string.h>

#include <list.h>
#include <loader.h>
#include <seq.h>


event* make_event(int tick, int type, int chan, int val1, int val2){
	event* e = malloc(sizeof(event));
	/* FIXME */
	return e;
}

int get_delta(reader* f){
  int a = read_byte(f);
  if(a<0x80){return a;}

  int b = read_byte(f);
  if(b<0x80){return ((a&0x7f)<<7) | b;}

  int c = read_byte(f);
  if(c<0x80){return ((a&0x7f)<<14) | ((b&0x7f)<<7) | c;}

  int d = read_byte(f);
  return ((a&0x7f)<<21) | ((b&0x7f)<<14) | ((c&0x7f)<<7) | d;
}

list* midi_load(char* filename){
return NULL;
	int i;
	reader* f = data_open("music/", filename);
	list* events = empty();

	char buf[16];
	char string[64];

	int uspq;
	int bpm;

	/*MThd*/
	loader_read(f,buf,4);

	/*0x00000006*/
	read_int(f);

	/*format type: 0x0000 0x0001 or 0x0002*/
	read_short(f);

	/*number of tracks*/
	short track_count = read_short(f);

	/* time division */
	loader_read(f,buf,2);
	//code to figure out time division

	for(i=0; i<track_count; i++){

		/*MTrk*/
		loader_read(f,buf,4);

		/* chunk size */
		int chunk_size = read_int(f);
		printf("%d\n",chunk_size);

		int tick = 0;
		int end_of_track = 0;
		int last_type = 0x80;
		int last_chan = 0;
		while(1){
			int delta = get_delta(f);

			if(delta < 0) return NULL;
			tick += delta;

			//type and channel
			buf[0] = read_byte(f);

			int type = buf[0] & 0xf0;
			if(type >= 0x80 && type <= 0xe0){//normal event
				last_type = type;
				int chan = buf[0] & 0x0f;
				last_chan = chan;
				loader_read(f,buf,2);
				int val1 = buf[0];
				int val2 = buf[1];
				append(events, make_event(tick, type, chan, val1, val2));
			}
			else if(type < 0x80){//running status
				int val1 = buf[0];
				buf[0] = read_byte(f);
				int val2 = buf[0];
				append(events, make_event(tick, last_type, last_chan, val1, val2));
			}
			else if(type == 0xff){//meta event
				buf[0] = read_byte(f);
				type = buf[0];

				int len = get_delta(f);

				switch(type){
					case 0x2f: printf("end of track\n");
						   end_of_track = 1;
						   break;
					case 0x51:printf("tempo change\n"); /*tempo*/
						  loader_read(f,buf,3);
						  uspq = (buf[0]<<16) | (buf[1]<<8) | buf[2];
						  bpm = 120;/*FIXME*/
						  break;
					case 0x01: printf("text\n");/*text*/
						   if(len >= 64){/*too big, skip ahead*/
							   loader_read(f, NULL, len);
						   }
						   else{
							   loader_read(f,string,len);
							   string[len] = '\0';
							   if(strncmp(string,"LoopStart",len)==0){
								push(events, make_event(tick, 0x100, 0, 1, 0));
							   }
							   else if(strncmp(string,"LoopEnd",len)==0){
								push(events, make_event(tick, 0x100, 0, 0, 0));
							   }
						   }
						   break;
					default: /*skip*/
						   loader_read(f,NULL,len);
						   break;
				}
			}
			else{ //sysex and such...
				int len = get_delta(f);
				loader_read(f, NULL, len);
			}

			if(end_of_track) break;
		}

	}


	//qsort(s->e, s->len, sizeof(event), event_cmp);
	//synth_setbpm(bpm);
	/* FIXME tempo change needs to be an event */

	return events;
}




