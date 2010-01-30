#define SFX_COUNT 256

typedef enum sfx_id {
	SFX_1,
	SFX_2,
	SFX_3,


	SFX_DONT_DEFINE_PAST_HERE = SFX_COUNT-1
} sfx_id;


int load_sound(char* filename, sfx_id id);
void play_sound(sfx_id id);
