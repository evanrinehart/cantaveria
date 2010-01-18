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
