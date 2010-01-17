enum input_type {
	BUTTON_PRESS,
	BUTTON_RELEASE,
	NO_INPUT,
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
	INVALID_BUTTON
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
const char* str_input(enum input_button, int player);
