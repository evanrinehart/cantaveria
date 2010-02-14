#include <stdlib.h>
#include <stdio.h>


#include <list.h>

list* freelist = NULL;


list* make_new(){
	/* FIXME */
	if(freelist == NULL){
		return malloc(sizeof(struct list));
	}
	else{
		list* ptr = NULL;
		return ptr;
	}
	return NULL;
}


list* empty(){
	return make_new();
}

void push(list* L, void* item){
	/* FIXME */
}

void* pop(list* L){
	/* FIXME */
	return NULL;
}

void append(list* L, void* item){
	/* FIXME */
}

void recycle(list* L){
	/* FIXME */
}
