#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

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

const char shapechars[20] = "Mmw12345678abcd";

char my_file[256] = "";
char my_file_old[256] = "";
char bgimage_file[256] = "";
char fgtiles_file[256] = "";
char bgtiles_file[256] = "";
char zone_path[256] = "";
int bgimage = 0;
int fgtiles = 0;
int bgtiles = 0;
int shapes = 0;
int tools = 0;

int select_enable = 0;
int select_x = 0;
int select_y = 0;
int select_w = 0;
int select_h = 0;

struct tile* raw_tiles = NULL;
int raw_w = 20;
int raw_h = 15;

int show_favorites = 0;
int bg_favorites[7] = {0,1,2,3,4,5,6};
int fg_favorites[7] = {0,1,2,3,4,5,6};
int brush_tile = 'M';
int brush_layer = 3;
int brush_enable = 0;
int erase_enable = 0;


int panic_flag = 0;

int dialog_flag = 0;
int background_dialog = 0;
int tileset_dialog = 0;
int quit_dialog = 0;
int save_as_dialog = 0;
int open_dialog = 0;
int confirm_save_dialog = 0;
int tools_dialog = 0;
int tile_panel = 0;



int tile_panel_set = 0;
int tile_panel_page = 0;
int tile_panel_offset = 0;

char save_as_buf[256] = "";
int save_as_ptr = 0;
char open_buf[256] = "";
int open_ptr = 0;

char gfx_path_buf[256] = "";
char stage_path_buf[256] = "";
/* *** */



/* utility */
void select_bgfile(char* path){
	strcpy(bgimage_file, path);
}

void set_zone_path(char* path){
	strncpy(zone_path, path, 256);
	zone_path[255] = 0;
	if(zone_path[strlen(zone_path)-1] != '/'){
		strncat(zone_path, "/", 256);
		zone_path[255] = 0;
	}
}

int file_exists(char* path){
	FILE* f = fopen(path, "r");
	if(f == NULL){
		return 0;
	}
	else{
		fclose(f);
		return 1;
	}
}

char* compute_stage_path(char* stage){
	strcpy(stage_path_buf, zone_path);
	strcat(stage_path_buf, "stages/");
	strcat(stage_path_buf, stage);
	return stage_path_buf;
}

char* compute_gfx_path(char* gfxfile){
	strcpy(gfx_path_buf, zone_path);
	strcat(gfx_path_buf, "gfx/");
	strcat(gfx_path_buf, gfxfile);
	return gfx_path_buf;
}
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
			gy = 16*(t.shape / 16);
			gx = 16*(t.shape % 16);
			if(toggle_shapes)
				draw_gfx_raw(shapes, i*16, j*16, gx, gy, 16, 16);
		}
	}

}

/* determine an optimal size for the stage */
void raw_optimize(int* ox, int* oy, int* ow, int* oh){
	int i;
	int xmax = 0;
	int xmin = INT_MAX;
	int ymax = 0;
	int ymin = INT_MAX;
	int x, y, fg, bg;
	char shape;

	for(i=0; i<(raw_w*raw_h); i++){
		x = i % raw_w;
		y = i / raw_w;
		fg = raw_tiles[i].fg;
		bg = raw_tiles[i].bg;
		shape = raw_tiles[i].shape;
		if((fg != 0 || bg != 0 || shape != '0')){
			if(x > xmax) xmax = x;
			if(x < xmin) xmin = x;
			if(y > ymax) ymax = y;
			if(y < ymin) ymin = y;
		}
	}

	if(ymax - ymin + 1 <= 15) *oh = 15;
	else *oh = (ymax - ymin + 1);

	if(xmax - xmin + 1 <= 20) *ow = 20;
	else *ow = (xmax - xmin + 1);

	*ox = xmin;
	*oy = ymin;
}

int raw_save(char* path){
	/* save current stage to a stage file */
	/* overwrites if already exists, no confirmation */
	int x, y, bg, fg;
	char shape;
	int i;
	struct tile* ptr = raw_tiles;
	int opt_ox, opt_oy;
	int opt_x, opt_y, opt_w, opt_h;


	FILE* f = fopen(path, "w");
	if(f == NULL){
		console_printf("error saving file");
		return -1;
	}

	raw_optimize(&opt_x, &opt_y, &opt_w, &opt_h);

	fprintf(f, "%d %d %d %d\n", opt_w, opt_h, origin_x-opt_x, origin_y-opt_y);
	fprintf(f, "%s\n", bgimage_file);
	fprintf(f, "%s\n", fgtiles_file);
	fprintf(f, "%s\n", bgtiles_file);

	for(i=0; i<(raw_w*raw_h); i++){
		x = (i % raw_w) - origin_x;
		y = (i / raw_w) - origin_y;
		fg = ptr[i].fg;
		bg = ptr[i].bg;
		shape = ptr[i].shape;

		if(fg != 0 || bg != 0 || shape != '0'){
			fprintf(f, "%d %d %d %d %c\n", x, y, fg, bg, shape);
		}
	}


	fclose(f);
	return 0;
}

void save(char* stagename){
	char* path = compute_stage_path(stagename);
	if(raw_save(path) < 0){
		console_printf("%s NOT saved", stagename);
	}
	else{
		console_printf("%s saved", stagename);
	}
}


int raw_open(char* stagename){
	reader* r;
	int w, h, ox, oy;
	int x, y, fg, bg;
	char shape;
	char file1[256] = "";
	char file2[256] = "";
	char file3[256] = "";
	struct tile* new_tiles = NULL;
	struct tile* ptr;
	char* path = compute_stage_path(stagename);

	r = loader_open(path);
	if(r == NULL){
		console_printf("Can't open %s", path);
		return -1;
	}

	if(loader_scanline(r, "%d %d %d %d", &w, &h, &ox, &oy) < 4){
		printf("scan error\n");
		loader_close(r);
		return -1;
	}

	if(
		loader_readline(r, file1, 256) ||
		loader_readline(r, file2, 256) ||
		loader_readline(r, file3, 256)
	){
		printf("scan error\n");
		loader_close(r);
		return -1;
	}

	new_tiles = initialize_raw(w, h);
	while(loader_scanline(r, "%d %d %d %d %c", &x, &y, &fg, &bg, &shape) == 5){
		ptr = new_tiles + (x+ox) + (y+oy)*w;
		ptr->fg = fg;
		ptr->bg = bg;
		ptr->shape = shape;
	}

	/* load the graphics */
	path = compute_gfx_path(file1);
	if(file1[0] != 0 && file_exists(path)) bgimage = load_bitmap(path);
	else bgimage = 0;

	path = compute_gfx_path(file2);
	if(file2[0] != 0 && file_exists(path)) bgtiles = load_bitmap(path);
	else bgtiles = 0;

	path = compute_gfx_path(file3);
	if(file3[0] != 0 && file_exists(path)) fgtiles = load_bitmap(path);
	else fgtiles = 0;

	/* finalize */
	origin_x = ox;
	origin_y = oy;
	raw_w = w;
	raw_h = h;
	free(raw_tiles);
	raw_tiles = new_tiles;

	strcpy(bgimage_file, file1);
	strcpy(bgtiles_file, file2);
	strcpy(fgtiles_file, file3);

	strcpy(my_file, path);

	return 0;
}

/* *** */



void update_window_name(){
	if(my_file[0] == 0){
		SDL_WM_SetCaption("unnamed", NULL);
	}
	else{
		SDL_WM_SetCaption(my_file, NULL);
	}
}



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
void update_favs(){
	int a, b, c, d, e;
	int *favs;
	int already = 0;
	int x = 0;
	int i = 0;
	int tmp;
	int value = brush_tile;
	int layer = brush_layer;

	if(layer == 1) favs = bg_favorites;
	if(layer == 2) favs = fg_favorites;

	for(i=0; i<7; i++){
		if(favs[i] == value){
			already = 1;
			x = i;
		}
	}

	if(already){
		if(x > 0){
			tmp = favs[x-1];
			favs[x-1] = favs[x];
			favs[x] = tmp;
		}
	}
	else{
		favs[6] = value;
	}

			
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


char* onoff(int b){
	if(b) return "on";
	else return "off";
}


/* *** */



/* dialog drawing */

void draw_tile_panel(){
	int i;
	int x, y;
	int gx, gy;
	int gfx;

	draw_black_rect(0,0,9*16,20*16);
	draw_gfx_raw(tools, 0, 14*16, 4*16, 0, 16, 16);
	draw_gfx_raw(tools, 16, 14*16, 5*16, 0, 16, 16);
	draw_gfx_raw(tools, 2*16, 14*16, 6*16, 0, 16, 16);
	draw_gfx_raw(tools, 7*16, 14*16, 7*16, 0, 16, 16);

	if(tile_panel_page == 0) tile_panel_offset = 0;
	if(tile_panel_page == 1) tile_panel_offset = 112;
	if(tile_panel_page == 2) tile_panel_offset = 144;

	if(tile_panel_set == 0) gfx = fgtiles;
	if(tile_panel_set == 1) gfx = bgtiles;

	for(i=0; i<112; i++){
		x = 16*(i % 8);
		y = 16*(i / 8);
		gx = 16*(i % 16);
		gy = 16*(i / 16);

		draw_gfx_raw(gfx, x, y, gx, gy+tile_panel_offset, 16, 16);
	}
}

void draw_tools(){
	int i;
	int gx, gy;

	draw_black_rect(16, 16, 17*16, 11*16);

	/* fg tile tool and favorites */
	draw_gfx_raw(tools, 2*16, 2*16, 0, 0, 16, 16);

	for(i=0; i<7; i++){
		gx = 16*(fg_favorites[i] % 16);
		gy = 16*(fg_favorites[i] / 16);
		draw_gfx_raw(fgtiles, 16*(i+4), 2*16, gx, gy, 16, 16);
	}

	/* bg tile tool and favorites */
	draw_gfx_raw(tools, 2*16, 4*16, 16, 0, 16, 16);

	for(i=0; i<7; i++){
		gx = 16*(bg_favorites[i] % 16);
		gy = 16*(bg_favorites[i] / 16);
		draw_gfx_raw(bgtiles, 16*(i+4), 4*16, gx, gy, 16, 16);
	}

	/* shapes */
	for(i=0; i<15; i++){
		gx = 16*(shapechars[i] % 16);
		gy = 16*(shapechars[i] / 16);
		draw_gfx_raw(shapes, 16*(i+2), 6*16, gx, gy, 16, 16);
	}

	draw_gfx_raw(tools, 2*16, 8*16, 2*16, 0, 16, 16);
	draw_gfx_raw(tools, 4*16, 8*16, 3*16, 0, 16, 16);
}


void redraw_all(){
	clear_video();
		
	draw_raw();

	//gui indicators
	if(select_enable){
		//draw green box
	}

	//dialogs
	if(save_as_dialog){
		console_printf("save as: %s", save_as_buf);
	}

	if(open_dialog){
		console_printf("open file: %s", open_buf);
	}

	if(tools_dialog){
		draw_tools();
	}

	if(tile_panel){
		draw_tile_panel();
	}

	console_draw();
	console_clear();

	update_video();
}
/* *** */





/* dialog input handlers */
void pixel_to_tile(int mx, int my, int* x, int* y){
	map_pixel(mx, my, x, y);
	*x /= 16;
	*y /= 16;
}

int tile_panel_click(int mx, int my){
	int x, y;

	pixel_to_tile(mx, my, &x, &y);

	if(x >= 9){
		return 0;
	}

	if(y == 14){
		if(x == 0){
			tile_panel_page = 0;
		}
		if(x == 1){
			tile_panel_page = 1;
		}
		if(x == 2){
			tile_panel_page = 2;
		}
		if(x == 7){
			tile_panel = 0;
		}
	}

	if(y >= 0 && y < 14){
		if(x >= 0 && x <=7){
			brush_tile = x + y*8 + tile_panel_offset;
			update_favs();
		}
	}

	redraw_all();
	return 1;
}

void tile_panel_press(SDLKey key, Uint16 c){
	switch(key){
		case SDLK_LEFT:
		case SDLK_RIGHT:
		case SDLK_UP:
		case SDLK_DOWN:
			return;
		default:
			tile_panel = 0;
			return;
	}
}

void tools_press(SDLKey key, Uint16 c){
	tools_dialog = 0;
}

void tools_click(int mx, int my){
	int x;
	int y;

	pixel_to_tile(mx, my, &x, &y);

	if(y == 2){
		if(x == 2){
			tile_panel = 1;
			tile_panel_set = 0;
			brush_layer = 2;
		}
		if(x >= 4 && x <= 10){
			brush_layer = 2;
			toggle_fgtiles = 1;
			toggle_shapes = 0;
			brush_tile = fg_favorites[x-4];
			update_favs();
		}
	}

	if(y == 4){
		if(x == 2){
			tile_panel = 1;
			tile_panel_set = 1;
			brush_layer = 1;
		}
		if(x >= 4 && x <= 10){
			brush_layer = 1;
			toggle_bgtiles = 1;
			toggle_shapes = 0;
			brush_tile = bg_favorites[x-4];
			update_favs();
		}
	}

	if(y == 6){
		if(x >= 2 && x <= 16){
			brush_layer = 3;
			toggle_shapes = 1;
			brush_tile = shapechars[x-2];
		}
	}

	if(y == 8){
		if(x == 2) printf("big eraser\n");
		if(x == 4) printf("eye dropper\n");
	}

	tools_dialog = 0;
	redraw_all();
}

void open_press(SDLKey key, Uint16 c){
	if(c == 0){
	}	
	else if(c == '\r'){
		if(open_buf[0] == 0){
			console_printf("No name? Nevermind then.");
		}
		else{
			if(raw_open(open_buf) < 0){
				console_printf("ERROR when opening %s", open_buf);
			}
			else {
				console_printf("%s opened", open_buf);
				update_window_name();
			}
		}
		open_buf[0] = 0;
		open_ptr = 0;
		open_dialog = 0;
	}
	else if(c == 0x1b){
		open_buf[0] = 0;
		open_ptr = 0;
		open_dialog = 0;
	}
	else if(c == '\b'){
		if(open_ptr > 0){
			open_ptr--;
			open_buf[open_ptr] = 0;
		}
	}
	else{
		if(open_ptr < 255){
			open_buf[open_ptr] = c;
			open_ptr++;
			open_buf[open_ptr] = 0;
		}
	}

}

void confirm_save_press(SDLKey key, Uint16 c){
	if(c == 'y' || c == 'Y'){
		console_printf("You're the boss. Overwriting %s", my_file);
		save(my_file);
		update_window_name();
	}
	else{
		strcpy(my_file, my_file_old); /* ! */
		console_printf("Operation cancelled");
	}

	confirm_save_dialog = 0;
}

void save_as_press(SDLKey key, Uint16 c){
	char* path;
	if(c == 0){
	}	
	else if(c == '\r'){
		if(save_as_buf[0] == 0){
			console_printf("No name? Nevermind then.");
		}
		else{
			strcpy(my_file_old, my_file); /* ! */
			strcpy(my_file, save_as_buf); /* ! */

			/* see if file exists */
			path = compute_stage_path(save_as_buf);
			if(file_exists(path)){
				console_printf("ALERT: really overwrite %s? (Y/N)", my_file);
				confirm_save_dialog = 1;
			}
			else{
				update_window_name();
				save(my_file);
			}
		}
		save_as_buf[0] = 0;
		save_as_ptr = 0;
		save_as_dialog = 0;
	}
	else if(c == 0x1b){
		save_as_buf[0] = 0;
		save_as_ptr = 0;
		save_as_dialog = 0;
	}
	else if(c == '\b'){
		if(save_as_ptr > 0){
			save_as_ptr--;
			save_as_buf[save_as_ptr] = 0;
		}
	}
	else{
		if(save_as_ptr < 255){
			save_as_buf[save_as_ptr] = c;
			save_as_ptr++;
			save_as_buf[save_as_ptr] = 0;
		}
	}
}

void quit_press(SDLKey key, Uint16 c){
	if(c == 'y' || c == 'Y'){
		panic_flag = 1;
	}
	else{
		console_printf("OK");
	}

	quit_dialog = 0;
}





void keydown(SDLKey key, SDLMod mod, Uint16 c){

	if(save_as_dialog){
		save_as_press(key, c);
		redraw_all();
		return;
	}

	if(confirm_save_dialog){
		confirm_save_press(key, c);
		redraw_all();
		return;
	}

	if(open_dialog){
		open_press(key, c);
		redraw_all();
		return;
	}

	if(quit_dialog){
		quit_press(key, c);
		redraw_all();
		return;
	}

	if(tools_dialog){
		tools_press(key, c);
		redraw_all();
		return;
	}

	if(tile_panel){
		tile_panel_press(key, c);
		redraw_all();
	}

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
				if(my_file[0] == 0){
					save_as_dialog = 1;
				}
				else{
					save(my_file);
				}
			}
			break;
		case SDLK_w:
			save_as_dialog = 1;
			break;
		case SDLK_o:
			open_dialog = 1;
			break;
		case SDLK_b:
			console_printf("change background...");
			break;
		case SDLK_q:
		case SDLK_ESCAPE:
			console_printf("Really quit? (Y/N)");
			quit_dialog = 1;
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

		case SDLK_SPACE:
			tools_dialog = 1;
			break;

		/* temporary controls */
		case SDLK_9: brush_tile--; brush_tile %= 256; break;
		case SDLK_0: brush_tile++; brush_tile %= 256; break;
		case SDLK_8: brush_layer = 2; break;
		case SDLK_7: brush_layer = 1; break;
	}

	redraw_all();

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

	if(tools_dialog){
		tools_click(mx, my);
		return;
	}

	if(tile_panel){
		if(tile_panel_click(mx, my)){
			return;
		}
	}


	if(button == 1){
		raw_write(x, y, brush_layer, brush_tile);
		brush_enable = 1;
		redraw_all();
	}
	else if(button == 3){
		erase_enable = 1;
		if(brush_layer == 3){
			raw_write(x, y, 3, '0');
		}
		else{
			raw_write(x, y, brush_layer, 0);
		}
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

	if(button == 3){
		erase_enable = 0;
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
	if(erase_enable){
		if(brush_layer == 3){
			raw_write(x, y, 3, '0');
		}
		else{
			raw_write(x, y, brush_layer, 0);
		}
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
		case SDL_KEYDOWN: keydown(e.key.keysym.sym, e.key.keysym.mod, e.key.keysym.unicode); return 0;
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

	update_window_name();

	shapes = load_bitmap("gfx/shapes.tga");
	tools = load_bitmap("gfx/tools.tga");

	loader_data_mode(0);
//	bgimage = load_bitmap("azone/gfx/background.tga");
//	loader_data_mode(0);
//	fgtiles = load_bitmap("barf.tga");
//	loader_data_mode(1);
//	fgtiles = load_bitmap("azone/gfx/barf.tga");
//	bgtiles = load_bitmap("azone/gfx/test.tga");

	set_zone_path("azone");

	redraw_all();

	SDL_ShowCursor(1);
	SDL_EnableUNICODE(1);
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

	atexit(terminate);

	while(check_events() == 0 && panic_flag == 0);

	return 0;
}
