/*
org.c
various software synths and samples

Copyright (C) 2009 Evan Rinehart

This software comes with no warranty.
1. This software can be used for any purpose, good or evil.
2. Modifications must retain this license, at least in spirit.
*/


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
ORG_FOO,
ORG_COOL
};



instrument load_instrument(enum instrument_name name);
