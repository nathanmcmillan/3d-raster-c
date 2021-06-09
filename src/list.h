/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef LIST_H
#define LIST_H

#include <stdbool.h>
#include <stdlib.h>

#include "mem.h"
#include "pie.h"

typedef struct List List;

struct List {
    void *item;
    List *next;
};

List *list_init();
void list_add(List *this, void *item);
void list_push(List *this, void *item);
void list_remove(List *this, void *item);
void list_remove_index(List *this, unsigned int index);
void list_insert(List *this, unsigned int index, void *item);
void list_insert_sort(List *this, int (*compare)(void *, void *), void *item);
void *list_find(List *this, bool(find)(void *, void *), void *has);
void *list_get(List *this, unsigned int index);
bool list_is_empty(List *this);
bool list_not_empty(List *this);
unsigned int list_size(List *this);
void **list_to_array(List *this);
void list_free(List *this);

#endif
