typedef struct list list;
struct list {
	void* item;
	list* next;
};

list* empty();
void push(list* L, void* item);
void* pop(list* L);
void append(list* L, void* item);
void recycle(list* L);

void list_sanitytest();
