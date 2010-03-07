/*
orc.c
various software synths and samples

Copyright (C) 2009 Evan Rinehart

This software comes with no warranty.
1. This software can be used for any purpose, good or evil.
2. Modifications must retain this license, at least in spirit.
*/

#define PI 3.1415926535897931
#define PI2 2*PI

enum {
	EV_NOTEON = 0x90,
	EV_NOTEOFF = 0x80,
	EV_CONTROLLER = 0xC0,
	EV_PITCHBEND = 0xE0
};

typedef void (*mix_callback)(void* data, float out[], int count);
typedef void (*control_callback)(void* data, int type, int val1, int val2, int val);
typedef void (*cleanup_callback)(void* data);

typedef struct {
	mix_callback mix;
	control_callback control;
	cleanup_callback cleanup;
	void* data;
} instrument;

enum instrument_name {
ORC_DEFAULT,
ORC_KARPLUS,
ORC_UNKNOWN
};



instrument orc_load(enum instrument_name name);
void orc_init(int sample_rate);
