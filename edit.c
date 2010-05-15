#include <stdio.h>
#include <stdlib.h>

#include <SDL/SDL.h>



#include <list.h>
#include <console.h>
#include <stage.h>
#include <loader.h>
#include <kernel.h>
#include <graphics.h>
#include <console.h>

struct edit {
	int x;
	int y;
	char shape;
	unsigned char fg;
	unsigned char bg;
	struct edit* next;
};

struct tile {
	char shape;
	unsigned char fg;
	unsigned char bg;
};

struct undo_step {
	struct tile* undo; /* writes to undo */
	struct tile* redo; /* writes to redo */
	struct edit* next;
	struct edit* prev;
};



/* application state variables */
int toggle_background = 1;
int toggle_bgtiles = 1;
int toggle_fgtiles = 1;
int toggle_shapes = 0;

int origin_x = 0;
int origin_y = 0;
static int camera_x = 0;
static int camera_y = 0;

char* my_filename = "";
char* bgimage_file = "";
char* fgtiles_file = "";
char* bgtiles_file = "";
int bgimage = 0;
int fgtiles = 0;
int bgtiles = 0;

int select_enable = 0;
int select_x = 0;
int select_y = 0;
int select_w = 0;
int select_h = 0;

struct tile* raw_tiles = NULL;
int raw_w = 20;
int raw_h = 15;

int show_favorites = 0;
int bg_favorites[7] = {0,0,0,0,0,0,0};
int fg_favorites[7] = {0,0,0,0,0,0,0};
int brush_tile = 1;
int brush_layer = 1;
int brush_enable = 0;

int panic_flag = 0;

int dialog_flag = 0;
int brush_dialog = 0;
int background_dialog = 0;
int tileset_dialog = 0;
int quit_dialog = 0;
int save_as_dialog = 0;
int open_dialog = 0;
/* *** */


/* base access methods */
struct tile* initialize_raw(int w, int h){
	int i;
	int j;
	struct tile blank = {'0', 0, 0};
	struct tile* ptr = malloc(w*h*sizeof(struct tile));
	for(j=0; j<h; j++){
		for(i=0; i<w; i++){
			*(ptr + i + j*w) = blank;
		}
	}

	return ptr;
}

void unload_raw(){
	free(raw_tiles);
}

struct tile raw_read(int x, int y){
	struct tile blank = {'0', 0, 0};
	if(x < 0 || y < 0 || x >= raw_w || y >= raw_h){
		return blank;
	}
	else{
		return *(raw_tiles + x + raw_w*y);
	}
}

void expand_raw(){
	int new_w = raw_w * 3;
	int new_h = raw_h * 3;
	struct tile* new_tiles = initialize_raw(new_w, new_h);
	struct tile* ptr;
	struct tile t;
	int i;
	int j;

	for(j=raw_h; j<2*raw_h; j++){
		for(i=raw_w; i<2*raw_w; i++){
			ptr = new_tiles + i + j*(3*raw_w);
			t = raw_read(i-raw_w, j-raw_h);
			*ptr = t;
		}
	}

	origin_x += raw_w;
	origin_y += raw_h;
	raw_w *= 3;
	raw_h *= 3;
	free(raw_tiles);
	raw_tiles = new_tiles;
}

int out_of_bounds(int x, int y){
	if(x < 0 || y < 0 || x >= raw_w || y >= raw_h)
		return 1;
	else
		return 0;
}

void detect_size(int* w, int* h){
//see the minimum size necessary for the area
//used for saving
}

void raw_write(int x, int y, int layer, int value){
	while(out_of_bounds(x, y)){
		printf("expanding\n");
		expand_raw();
		x += raw_w / 3;
		y += raw_h / 3;
	}

	//expand if outside
	//shift x y by expand shift
	//shift x and y by origin
	//do the write
	struct tile* ptr = raw_tiles + x + raw_w*y;
	if(layer == 1){
		ptr->bg = value;
	}
	else if(layer == 2){
		ptr->fg = value;
	}
	else if(layer == 3){
		ptr->shape = value;
	}
}

void draw_background(){
	int W = gfx_width(bgimage);
	int H = gfx_height(bgimage);
	draw_gfx_raw(bgimage, 0, 0, 0, 0, W, H);
}

void draw_raw(){
	int x0 = camera_x + origin_x;
	int y0 = camera_y + origin_y;
	int i;
	int j;
	int x;
	int y;
	struct tile t;
	int gx;
	int gy;

	if(toggle_background)
		draw_background();

	for(j=0; j<(15+5); j++){
		y = y0 + j;
		for(i=0; i<(20+8); i++){
			x = x0 + i;
			t = raw_read(x, y);
			gy = 16*(t.bg / 16);
			gx = 16*(t.bg % 16);
			if(toggle_bgtiles)
				draw_gfx_raw(bgtiles, i*16, j*16, gx, gy, 16, 16);
			gy = 16*(t.fg / 16);
			gx = 16*(t.fg % 16);
			if(toggle_fgtiles)
				draw_gfx_raw(fgtiles, i*16, j*16, gx, gy, 16, 16);
		}
	}

}

/* *** */





struct undo_step* undo_stack;
struct undo_step* undo_ptr;

/* undo operations */
void undo(){
//do the undo_ptr->undo operations
//move undo_ptr down one
}

void redo(){
//if at top of stack, do nothing

//do the undo_ptr->redo operations
//move undo_ptr up one
}

void undo_record(struct edit* edits){
//eliminate undo_ptr->redo and all previous edit structs
//change the undo_stack

//store the edits in undo_ptr->redo
//calculate the undo operation XXX
//push a new edit struct
//move undo_ptr
//store the undo operation in undo_ptr->undo
}
/* *** */




/* medium level editting commands */
void write_one_tile(int x, int y, int layer, int value){
//write x y layer value
}

void write_many_tiles(struct edit* edits){
//for each tiles
//write one tile
}

void edit_one_tile(int x, int y, int layer, int value){
//write_one_tile
//create a tile struct
//call edit on it
}

void edit_many_tiles(struct edit* edits){
//write many tiles
//edit(tiles)
}

void add_to_clipboard(struct edit* edits){
//makes a tile struct and appends to clipboard
}

void clear_clipboard(){
//clear the clipboard
}

struct tile* read_tile(int x, int y){
//make a tile struct
}
/* *** */



/* high level gui commands */
void select_brush(int layer, int value){

}

void start_box(int x, int y){

}

void move_box(int x, int y){

}

void stop_box(){

}

void clear_box(){

}

void append_to_box(int x, int y){

}

struct tile* box_select(){

}


void move_paste(int x, int y){

}

void cancel_paste(){

}

void do_paste(){

}


void redraw_all(){
	clear_video();
		
	draw_raw();

	//gui indicators
	if(select_enable){
		//draw green box
	}

	//dialogs

	console_draw();

	update_video();
}

char* onoff(int b){
	if(b) return "on";
	else return "off";
}



void save(){
	printf("save code here\n");
}
/* *** */














void keydown(SDLKey key, SDLMod mod){
	switch(key){
		case SDLK_u:
			undo();
			console_printf("undo"); break;
		case SDLK_r:
			redo();
			console_printf("redo"); break;
		case SDLK_1:
			toggle_background = !toggle_background;
			console_printf("background %s", onoff(toggle_background));
			break;
		case SDLK_2:
			toggle_bgtiles = !toggle_bgtiles;
			console_printf("bg tiles %s", onoff(toggle_bgtiles));
			break;
		case SDLK_3:
			toggle_fgtiles = !toggle_fgtiles;
			console_printf("fg tiles %s", onoff(toggle_fgtiles));
			break;
		case SDLK_4:
			toggle_shapes = !toggle_shapes;
			console_printf("shapes %s", onoff(toggle_shapes));
			break;
		case SDLK_s:
			if(mod & (KMOD_LCTRL|KMOD_RCTRL)){
				save();
				console_printf("saved %s", my_filename);
			}
			break;
		case SDLK_w:
			console_printf("save as...");
			break;
		case SDLK_o:
			console_printf("open...");
			break;
		case SDLK_b:
			console_printf("change background...");
			break;
		case SDLK_q:
			if(dialog_flag == 0){
				dialog_flag = 1;
				quit_dialog = 1;
			}
			break;
		case SDLK_ESCAPE:
			panic_flag = 1;
			if(dialog_flag == 0){
				dialog_flag = 1;
				quit_dialog = 1;
			}
			else{
				dialog_flag = 0;
			}
			break;
		case SDLK_RETURN: 
			console_printf("OK");
			break;
		case SDLK_y:
			console_printf("yes");
			break;
		case SDLK_n:
			console_printf("no");
			break;
		case SDLK_h:
		case SDLK_F1:
		case SDLK_SLASH:
			console_printf("help...");
			break;
		case SDLK_F2:
			console_printf("pick fg tileset...");
			break;
		case SDLK_F3:
			console_printf("pick bg tileset...");
			break;
		case SDLK_LEFT: camera_x--; break;
		case SDLK_RIGHT: camera_x++; break;
		case SDLK_UP: camera_y--; break;
		case SDLK_DOWN: camera_y++; break;
	}

	redraw_all();

	console_clear();
/*
U - undo
R - redo
1 - toggle layer 1
2 - toggle layer 2
3 - toggle layer 3
4 - toggle layer 4
ctrl S - save
W - save as
O - open stage
B - change background
Q - quit
ESC - quit / cancel
ENTER - yes / OK
Y - yes
N - no
H - help
? - help
F1 - help
F2 - change fg tileset
F3 - change bg tileset
ARROW KEYS - scroll
*/
}

void translate_pointer(int mx, int my, int *x, int *y){
	int a, b;
	map_pixel(mx, my, &a, &b);
	*x = a/16 + camera_x + origin_x;
	*y = b/16 + camera_y + origin_y;
}


void mousedown(int mx, int my, int button){
/*
hold LMB - draw single tiles / deselect
shift LMB - start box select
ctrl LMB - append single tiles to selection
RMB - display tilesets
hold MMB - choose where to paste (release to execute, esc to cancel)
*/
	SDLMod mod = SDL_GetModState();

	int x, y;
	translate_pointer(mx, my, &x, &y);

	if(brush_dialog){
		//change brush, maybe
		brush_dialog = 0;
		return;
	}

	


	if(button == 1){
		raw_write(x, y, brush_layer, brush_tile);
		brush_enable = 1;
		redraw_all();
	}
	else if(button == 3){
		brush_dialog = 1;
		redraw_all();
	}

}

void mouseup(int x, int y, int button){
/*
LMB - stop drawing
shift LMB - append box to selection
MMB - execute paste
*/

	if(button == 1){
		brush_enable = 0;
	}
}


void mousemove(int mx, int my, int xrel, int yrel){
/*
redraw cursor
redraw box select
redraw paste box
*/
	int x, y;
	translate_pointer(mx, my, &x, &y);

	if(brush_enable){
		raw_write(x, y, brush_layer, brush_tile);
		redraw_all();
	}
}

int check_events(){
	SDL_Event e;

	if(SDL_WaitEvent(&e) == 0){
		printf("SDL_WaitEvent encountered an error (%s)\n", SDL_GetError());
		return 1;
	}

	switch(e.type){
		case SDL_QUIT: return 1;
		case SDL_KEYDOWN: keydown(e.key.keysym.sym, e.key.keysym.mod); return 0;
		case SDL_MOUSEMOTION:			
			mousemove(e.motion.x, e.motion.y, e.motion.xrel, e.motion.yrel);
			return 0;
		case SDL_MOUSEBUTTONDOWN: mousedown(e.button.x, e.button.y, e.button.button); return 0;
		case SDL_MOUSEBUTTONUP: mouseup(e.button.x, e.button.y, e.button.button); return 0;
		default: return 0;
	}
}



void terminate(){
	loader_quit();
	video_quit();
}

int main(int argc, char* argv[]){
	video_init(argc, argv);
	loader_init();
	graphics_init();

	raw_tiles = initialize_raw(raw_w, raw_h);
	bgimage = load_bitmap("bgimages/background.tga");
	fgtiles = load_bitmap("tilesets/test.tga");
	bgtiles = load_bitmap("tilesets/test.tga");

	raw_write(2, 2, 1, 3);

	redraw_all();

	SDL_ShowCursor(1);
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

	atexit(terminate);

	while(check_events() == 0 && panic_flag == 0);

	return 0;
}
