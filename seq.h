typedef struct event event;
struct event {
	int tick;
	char midi[4];
	struct event* next;
};

void seq_init();

int seq_lookahead(int samples);
event* seq_get_event();
void seq_advance(int samples);


