/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef ARRAY_H
#define ARRAY_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "mem.h"

typedef struct array array;

struct array {
    void **items;
    unsigned int length;
    unsigned int capacity;
};

bool find_address(void *item, void *has);

void array_init_with_capacity(array *this, unsigned int length, unsigned int capacity);
void array_init(array *this, unsigned int length);

array *create_array_with_capacity(unsigned int length, unsigned int capacity);
array *create_array(unsigned int length);
array *create_array_with_items(unsigned int length, unsigned int capacity, void **items);

void **array_copy_items(array *this);
array *create_array_copy(array *this);

void array_push(array *this, void *item);
void array_insert(array *this, unsigned int index, void *item);
void array_insert_sort(array *this, int (*compare)(void *, void *), void *item);

void *array_find(array *this, bool(find)(void *, void *), void *has);
void *array_get(array *this, unsigned int index);

void *array_pop(array *this);
void array_remove(array *this, void *item);
void array_remove_index(array *this, unsigned int index);

void array_clear(array *this);

bool array_is_empty(array *this);
bool array_not_empty(array *this);
unsigned int array_size(array *this);

void release_array(array *this);
void delete_array(array *this);

#endif
