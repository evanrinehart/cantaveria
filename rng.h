/*
rng.c
a random number generator

Copyright (C) 2009 Evan Rinehart

This software comes with no warranty.
1. This software can be used for any purpose, good or evil.
2. Modifications must retain this license, at least in spirit.
*/

unsigned zrand();
void zseed(unsigned data[], unsigned carry);
void zread(unsigned data[], unsigned* carry);
void zreset();
void zsrand(unsigned s);
void zsrand_u();

