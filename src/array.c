/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "array.h"

bool find_address(void *item, void *has) {
    return item == has;
}

static void init_with_capacity(Array *this, int size, int capacity) {
    if (capacity == 0) {
        this->items = NULL;
    } else {
        this->items = Calloc(capacity, sizeof(void *));
    }
    this->size = size;
    this->capacity = capacity;
}

Array *NewArrayWithCapacity(int size, int capacity) {
    Array *this = Malloc(sizeof(Array));
    init_with_capacity(this, size, capacity);
    return this;
}

Array *NewArray(int size) {
    return NewArrayWithCapacity(size, size);
}

void **array_copy_items(Array *this) {
    int size = this->size * sizeof(void *);
    void **copy = Malloc(size);
    memcpy(copy, this->items, size);
    return copy;
}

Array *new_array_copy(Array *from) {
    Array *this = Malloc(sizeof(Array));
    this->items = array_copy_items(from);
    this->size = from->size;
    this->capacity = from->size;
    return this;
}

static void update_capacity(Array *this, int size) {
    if (size > this->capacity) {
        if (this->capacity == 0) {
            this->capacity = size;
            this->items = Calloc(size, sizeof(void *));
        } else {
            this->capacity = size * 2;
            this->items = Realloc(this->items, this->capacity * sizeof(void *));
            memset(this->items + this->size, 0, this->capacity - this->size);
        }
    }
}

void ArrayPush(Array *this, void *item) {
    int size = this->size + 1;
    update_capacity(this, size);
    this->size = size;
    this->items[size - 1] = item;
}

void ArrayInsert(Array *this, int index, void *item) {
    int size = this->size + 1;
    update_capacity(this, size);
    this->size = size;
    void **items = this->items;
    for (int i = size - 1; i > index; i--) {
        items[i] = items[i - 1];
    }
    items[index] = item;
}

void array_insert_sort(Array *this, int (*compare)(void *, void *), void *item) {
    int len = this->size;
    void **items = this->items;
    for (int i = 0; i < len; i++) {
        if (compare(item, items[i]) <= 0) {
            ArrayInsert(this, i, item);
            return;
        }
    }
    ArrayPush(this, item);
}

void *array_find(Array *this, bool(find)(void *, void *), void *has) {
    int len = this->size;
    void **items = this->items;
    for (int i = 0; i < len; i++) {
        if (find(items[i], has)) {
            return items[i];
        }
    }
    return NULL;
}

void *ArrayGet(Array *this, int index) {
    if (index >= this->size) {
        return NULL;
    }
    return this->items[index];
}

void *array_pop(Array *this) {
    if (this->size == 0) {
        return NULL;
    }
    this->size--;
    return this->items[this->size];
}

void array_remove(Array *this, void *item) {
    int len = this->size;
    void **items = this->items;
    for (int i = 0; i < len; i++) {
        if (items[i] == item) {
            len--;
            while (i < len) {
                items[i] = items[i + 1];
                i++;
            }
            this->size--;
            items[len] = NULL;
            return;
        }
    }
}

void array_remove_index(Array *this, int index) {
    this->size--;
    int len = this->size;
    void **items = this->items;
    for (int i = index; i < len; i++) {
        items[i] = items[i + 1];
    }
    items[len] = NULL;
}

void ArrayClear(Array *this) {
    this->size = 0;
}

bool ArrayIsEmpty(Array *this) {
    return this->size == 0;
}

bool ArrayNotEmpty(Array *this) {
    return this->size != 0;
}

int ArraySize(Array *this) {
    return this->size;
}

void ArrayFree(Array *this) {
    Free(this->items);
    Free(this);
}
