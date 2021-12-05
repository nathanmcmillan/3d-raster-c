/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "array.h"

bool find_address(void *item, void *has) {
    return item == has;
}

void array_init_with_capacity(Array *this, usize length, usize capacity) {
    if (capacity == 0) {
        this->items = NULL;
    } else {
        this->items = Calloc(capacity, sizeof(void *));
    }
    this->length = length;
    this->capacity = capacity;
}

void array_init(Array *this, usize length) {
    array_init_with_capacity(this, length, length);
}

Array *new_array_with_capacity(usize length, usize capacity) {
    Array *this = Malloc(sizeof(Array));
    array_init_with_capacity(this, length, capacity);
    return this;
}

Array *new_array(usize length) {
    return new_array_with_capacity(length, length);
}

Array *new_array_with_items(usize length, usize capacity, void **items) {
    Array *this = Malloc(sizeof(Array));
    this->items = items;
    this->length = length;
    this->capacity = capacity;
    return this;
}

void **array_copy_items(Array *this) {
    usize size = this->length * sizeof(void *);
    void **copy = Malloc(size);
    memcpy(copy, this->items, size);
    return copy;
}

Array *new_array_copy(Array *from) {
    Array *this = Malloc(sizeof(Array));
    this->items = array_copy_items(from);
    this->length = from->length;
    this->capacity = from->length;
    return this;
}

static void update_capacity(Array *this, usize length) {
    if (length > this->capacity) {
        if (this->capacity == 0) {
            this->capacity = length;
            this->items = Calloc(length, sizeof(void *));
        } else {
            this->capacity = length * 2;
            this->items = Realloc(this->items, this->capacity * sizeof(void *));
            memset(this->items + this->length, 0, this->capacity - this->length);
        }
    }
}

void array_push(Array *this, void *item) {
    usize length = this->length + 1;
    update_capacity(this, length);
    this->length = length;
    this->items[length - 1] = item;
}

void array_insert(Array *this, usize index, void *item) {
    usize length = this->length + 1;
    update_capacity(this, length);
    this->length = length;
    void **items = this->items;
    for (usize i = length - 1; i > index; i--) {
        items[i] = items[i - 1];
    }
    items[index] = item;
}

void array_insert_sort(Array *this, int (*compare)(void *, void *), void *item) {
    usize len = this->length;
    void **items = this->items;
    for (usize i = 0; i < len; i++) {
        if (compare(item, items[i]) <= 0) {
            array_insert(this, i, item);
            return;
        }
    }
    array_push(this, item);
}

void *array_find(Array *this, bool(find)(void *, void *), void *has) {
    usize len = this->length;
    void **items = this->items;
    for (usize i = 0; i < len; i++) {
        if (find(items[i], has)) {
            return items[i];
        }
    }
    return NULL;
}

void *ArrayGet(Array *this, usize index) {
    if (index >= this->length) {
        return NULL;
    }
    return this->items[index];
}

void *array_pop(Array *this) {
    if (this->length == 0) {
        return NULL;
    }
    this->length--;
    return this->items[this->length];
}

void array_remove(Array *this, void *item) {
    usize len = this->length;
    void **items = this->items;
    for (usize i = 0; i < len; i++) {
        if (items[i] == item) {
            len--;
            while (i < len) {
                items[i] = items[i + 1];
                i++;
            }
            this->length--;
            items[len] = NULL;
            return;
        }
    }
}

void array_remove_index(Array *this, usize index) {
    this->length--;
    usize len = this->length;
    void **items = this->items;
    for (usize i = index; i < len; i++) {
        items[i] = items[i + 1];
    }
    items[len] = NULL;
}

void array_clear(Array *this) {
    this->length = 0;
}

bool array_is_empty(Array *this) {
    return this->length == 0;
}

bool array_not_empty(Array *this) {
    return this->length != 0;
}

usize array_size(Array *this) {
    return this->length;
}

void array_release(Array *this) {
    Free(this->items);
}

void array_delete(Array *this) {
    array_release(this);
    Free(this);
}
