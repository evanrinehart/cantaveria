void setup_inner(void);
void setup_splash(void);
void setup_intro(void);
void setup_title(void);

void set_handler(
	void (*update)(void),
	void (*draw)(void),
	void (*press)(input in),
	void (*release)(input in)
);

