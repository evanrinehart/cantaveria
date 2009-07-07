#define MAX_SPRITES 256
#define MAX_ANIMATIONS 256

struct frame{
  int x, y;
  double x0, y0, x1, y1;
};

typedef struct sprite sprite;
struct sprite {
  int number;
  int frame_counter;
  int current_frame;
  struct frame frame;
  int gfxid;
  int anim;
  int x, y, w, h, vx, vy;
  void (*update)(sprite*, void* ud);
  void* userdata;
};

typedef struct {
  int w, h;
  int loop_mode;/*0 stopped, 1 once, 2 repeat, 3 pingpong, 4 evaporate*/
  int gfxid;
  int frame_count;
  short* frame_lens;
  struct frame* frames;
} animation;

void graphics_init();

void draw();

void draw_small_text(char* str, int x, int y);

void printf_small(int x, int y, char* format, ...);


int load_sprite(char* filename, int id);
int load_font(char* filename);
sprite* enable_sprite(int sprnum);
void disable_sprite(sprite* spr);
sprite* copy_sprite(sprite* spr);
void point_camera(int x, int y);

void draw_small_text(char* str, int x, int y);
void printf_small(int x, int y, char* format, ...);


void set_message(char* str);
void advance_message();
void clear_message();
void complete_message();

void reposition_message(int x, int y);
void resize_message(int w, int h);

void animate_sprites();


void enable_stage(int yn);
