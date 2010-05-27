typedef struct list list;
struct list {
	void* item;
	list* next;
};

typedef int (*compare_func)(void* v1, void* v2);

list* empty(void);
void push(list* L, void* item);
void* pop(list* L);
void append(list* L, void* item);
void recycle(list* L);
void print_list(list* L, void (*print)(void* item));
int length(list* L);
void sort(list* L, compare_func cmp);
