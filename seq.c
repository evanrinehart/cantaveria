#include <stdlib.h>

#include <seq.h>

static struct {
	int tick;
	int terr; // number 1/1323000 ticks

	int loop_start;
	int loop_end;
	int looping;

	event* next_event;
	event* events;
} my;

void seq_init(){

}

int would_loop(){
	return my.looping && my.next_event->tick > my.loop_end;
}

event* get_event_after(int tick){
	return NULL;
}

event* get_next_event(){
	if(would_loop()){
		return get_event_after(my.loop_start);
	}
	else {
		return my.next_event;
	}
}



int get_next_tick(){
	event* e = get_next_event();
	return e ? e->tick : -1;
}

int distance_to_next(){
	int next_tick = get_next_tick();
	if(next_tick < 0) return -1;

	return would_loop() ?
		next_tick - my.loop_start + my.loop_end - my.tick :
		next_tick - my.tick;

}


/* below are three functions that the synth uses to
control the sequencer. it finds control events, advances
the event pointer, and finally advances the tick count */

//returns samples from now an event will occur
//if no event will occur in sbound samples, returns -1
int seq_lookahead(int sbound){
return -1;
	int tbound = sbound*46080/1323000;
	int T = distance_to_next();
	if(T < 0) return -1;
	return T > tbound ?
		-1 :
		T*1 + 0;
}

//returns the next event that would play
//if there is no such event, returns NULL
event* seq_get_event(){
return NULL;
	//needs to handle looping
}


//advance the tick position
void seq_advance(int samples){
return;
	// 46080 1/1323000 ticks = 1 sample
	int N = 46080 * samples;
	int D = 1323000;
	my.terr += N;
	my.tick += N/D + my.terr/D;
	my.terr %= D;
//needs to handle looping
}


/* IMPORTANT
it might well be simpler to implement looping as
an event which exists at the loop point after all
other events that occur at that time which sends
the sequence to a specific tick */
