void setup_inner();
void setup_splash();
void setup_intro();
void setup_title();

void set_handler(
	void (*update)(),
	void (*draw)(),
	void (*press)(input in),
	void (*release)(input in)
);

