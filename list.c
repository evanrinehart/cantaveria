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
	void* item = L->next->item;
	remove_from_list(L, L->next);
	return item;
}

void append(list* L, void* item){
	list* ptr = L->next;
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



void list_print(list* L){
	list* ptr = L->next;
	printf("(");
	while(ptr){
		char* s = ptr->item;
		printf("%s", s);
		if(ptr->next)
			printf(", ");
		ptr = ptr->next;
	}
	printf(")\n");
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

void list_sanitytest(){
	list* L = empty();
	char* s;
	printf("empty: ");
	list_print(L);

	printf("push a b c: ");
	push(L, "a");
	push(L, "b");
	push(L, "c");
	list_print(L);

	printf("pop: ");
	s = pop(L);
	printf("%s ", s);
	list_print(L);

	printf("freelist: ");
	list_print_free();
	
	printf("pop: ");
	s = pop(L);
	printf("%s ", s);
	list_print(L);

	printf("freelist: ");
	list_print_free();

	printf("append a b c: ");
	append(L, "a");
	append(L, "b");
	append(L, "c");
	list_print(L);
	
	printf("freelist: ");
	list_print_free();

	printf("recycle: ");
	recycle(L);
	list_print_free();
	
}
