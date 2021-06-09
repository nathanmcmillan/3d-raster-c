#ifndef LIST_H
#define LIST_H

#include <stdbool.h>
#include <stdlib.h>

#include "mem.h"
#include "pie.h"

typedef struct list list;

struct list {
    void *item;
    list *next;
};

list *list_init();
void list_add(list *this, void *item);
void list_push(list *this, void *item);
void list_remove(list *this, void *item);
void list_remove_index(list *this, unsigned int index);
void list_insert(list *this, unsigned int index, void *item);
void list_insert_sort(list *this, int (*compare)(void *, void *), void *item);
void *list_find(list *this, bool(find)(void *, void *), void *has);
void *list_get(list *this, unsigned int index);
bool list_is_empty(list *this);
bool list_not_empty(list *this);
unsigned int list_size(list *this);
void **list_to_array(list *this);
void list_free(list *this);

#endif
