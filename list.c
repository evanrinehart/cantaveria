#include <stdlib.h>
#include <stdio.h>


#include <list.h>

list* freelist = NULL;


list* make_new(list* next, void* item){
	list* ptr;

	if(freelist == NULL){
		ptr = malloc(sizeof(struct list));
	}
	else{
		ptr = freelist;
		freelist = ptr->next;
	}

	ptr->next = next;
	ptr->item = item;
	return ptr;
}

void remove_from_list(list* prev, list* node){
	prev->next = node->next;
	node->item = NULL;
	node->next = freelist;
	freelist = node;
}


list* empty(){
	return make_new(NULL, NULL);
}

void push(list* L, void* item){
	list* ptr = make_new(L->next, item);
	L->next = ptr;
}

void* pop(list* L){
	if(L->next){
		void* item = L->next->item;
		remove_from_list(L, L->next);
		return item;
	}
	else{
		return NULL;
	}

}

void append(list* L, void* item){
	list* ptr = L;
	while(ptr->next){
		ptr = ptr->next;
	}
	ptr->next = make_new(NULL, item);
}

void recycle(list* L){
	list* ptr = L;
	while(ptr->next){
		ptr->next->item = NULL;
		ptr = ptr->next;
	}
	ptr->next = freelist;
	freelist = L;
}

int length(list* L){
	list* ptr = L->next;
	int c = 0;
	while(ptr){
		c += 1;
		ptr = ptr->next;
	}
	return c;
}




void print_list(list* L, void (*print)(void* item)){
	list* ptr = L->next;
	printf("(");
	while(ptr){
		print(ptr->item);
		if(ptr->next)
			printf(", ");
		ptr = ptr->next;
	}
	printf(")");
}

void println_list(list* L, void (*print)(void* item)){
	print_list(L, print);
	printf("\n");
}

void list_print_free(){
	list* ptr = freelist;
	printf("(");
	while(ptr){
		printf("_");
		if(ptr->next){
			printf(", ");
		}
		ptr = ptr->next;
	}
	printf(")\n");
}

void print(void* item){
	char* s = item;
	printf("%s", s);
}

