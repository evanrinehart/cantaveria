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

typedef enum {
	MUS_NOTHING,
	MUS_COOL
} mus_id;

int music_load(char* filename, mus_id id);
void music_unload(mus_id id);
mus_id music_current();

void music_play(mus_id id);
void music_stop(mus_id id);
void music_pause();

void music_volume(int precent);
void music_fadeout(int seconds);

/*
the music player

int music_load(filename, id)
Attempts to load song in filename into song slot id. Returns
0 if successful and -1 otherwise. If there is a song already
in slot id, it will fail.

void music_unload(id)
unloads the song in slot id. if no song is in that position
the call fails silently. after this operation you can
load a new song in that position.

id music_current()
return the id of the currently playing song.

void music_play(id)
play the song with id id. if there is no such song loaded, the
operation will fail silently. if there is such a song but another
is playing, that song will be paused immediately.

void music_stop(id)
stop and reset the song with id id. if there is no such song, the
operation will fail silently. if the song is currently playing, it
will be stopped immediately.

void music_pause()
pause the currently playing song. if no song is playing, fails
silently. after pausing, music_play on that song will resume at
the previous position.

void music_volume(percent)
additional volume adjustment for all songs.

void music_fadeout(seconds)
fade out the current song over seconds seconds. if the fade out
completes, the song is stopped and reset. fadeout can be cancelled
by music_play on any song, music_stop on the current song, another
calls to music_fadeout, or music_pause. in this case the fade is
cancelled and reset. basically any call to fadeout will cause a
new fadeout effect.

*/
