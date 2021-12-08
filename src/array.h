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
    int size;
    int capacity;
};

bool find_address(void *item, void *has);

Array *NewArrayWithCapacity(int size, int capacity);
Array *NewArray(int size);

void **array_copy_items(Array *this);
Array *new_array_copy(Array *this);

void ArrayPush(Array *this, void *item);
void ArrayInsert(Array *this, int index, void *item);
void array_insert_sort(Array *this, int (*compare)(void *, void *), void *item);

void *array_find(Array *this, bool(find)(void *, void *), void *has);
void *ArrayGet(Array *this, int index);

void *array_pop(Array *this);
void array_remove(Array *this, void *item);
void array_remove_index(Array *this, int index);

void ArrayClear(Array *this);

bool ArrayIsEmpty(Array *this);
bool ArrayNotEmpty(Array *this);
int ArraySize(Array *this);

void ArrayFree(Array *this);

#endif
