#include <stdlib.h>

#include <list.h>
#include <loader.h>
#include <sfx.h>

float* sfx[SFX_COUNT];
int sfx_len[SFX_COUNT];


int load_sound(char* filename, sfx_id id){
	/* load a wave file */
	reader* r = data_open("sounds", filename);
	if(r==NULL){
		return -1;
	}

	int i;
	for(i=0; i<40; i++){
		read_byte(r);
	}

	int N = read_int(r) / 2;
	float* buffer = malloc(N);

	for(i=0; i<N; i++){
		int s = read_short(r);
		buffer[i] = s / 32767.0f;
	}

	sfx[id] = buffer;
	sfx_len[id] = N;

	loader_close(r);
	return 0;
}

void play_sound(sfx_id id){

}
