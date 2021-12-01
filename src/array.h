/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef ARRAY_H
#define ARRAY_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "mem.h"
#include "super.h"

typedef struct Array Array;

struct Array {
    void **items;
    usize length;
    usize capacity;
};

bool find_address(void *item, void *has);

void array_init_with_capacity(Array *this, usize length, usize capacity);
void array_init(Array *this, usize length);

Array *new_array_with_capacity(usize length, usize capacity);
Array *new_array(usize length);
Array *new_array_with_items(usize length, usize capacity, void **items);

void **array_copy_items(Array *this);
Array *new_array_copy(Array *this);

void array_push(Array *this, void *item);
void array_insert(Array *this, usize index, void *item);
void array_insert_sort(Array *this, int (*compare)(void *, void *), void *item);

void *array_find(Array *this, bool(find)(void *, void *), void *has);
void *array_get(Array *this, usize index);

void *array_pop(Array *this);
void array_remove(Array *this, void *item);
void array_remove_index(Array *this, usize index);

void array_clear(Array *this);

bool array_is_empty(Array *this);
bool array_not_empty(Array *this);
usize array_size(Array *this);

void array_release(Array *this);
void array_delete(Array *this);

#endif
