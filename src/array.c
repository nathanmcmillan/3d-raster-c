/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "array.h"

bool find_address(void *item, void *has) {
    return item == has;
}

void array_init_with_capacity(Array *this, unsigned int length, unsigned int capacity) {
    if (capacity == 0) {
        this->items = NULL;
    } else {
        this->items = safe_calloc(capacity, sizeof(void *));
    }
    this->length = length;
    this->capacity = capacity;
}

void array_init(Array *this, unsigned int length) {
    array_init_with_capacity(this, length, length);
}

Array *create_array_with_capacity(unsigned int length, unsigned int capacity) {
    Array *this = safe_malloc(sizeof(Array));
    array_init_with_capacity(this, length, capacity);
    return this;
}

Array *create_array(unsigned int length) {
    return create_array_with_capacity(length, length);
}

Array *create_array_with_items(unsigned int length, unsigned int capacity, void **items) {
    Array *this = safe_malloc(sizeof(Array));
    this->items = items;
    this->length = length;
    this->capacity = capacity;
    return this;
}

void **array_copy_items(Array *this) {
    size_t size = this->length * sizeof(void *);
    void **copy = safe_malloc(size);
    memcpy(copy, this->items, size);
    return copy;
}

Array *create_array_copy(Array *from) {
    Array *this = safe_malloc(sizeof(Array));
    this->items = array_copy_items(from);
    this->length = from->length;
    this->capacity = from->length;
    return this;
}

static void update_capacity(Array *this, unsigned int length) {
    if (length > this->capacity) {
        if (this->capacity == 0) {
            this->capacity = length;
            this->items = safe_calloc(length, sizeof(void *));
        } else {
            this->capacity = length * 2;
            this->items = safe_realloc(this->items, this->capacity * sizeof(void *));
            memset(this->items + this->length, 0, this->capacity - this->length);
        }
    }
}

void array_push(Array *this, void *item) {
    unsigned int length = this->length + 1;
    update_capacity(this, length);
    this->length = length;
    this->items[length - 1] = item;
}

void array_insert(Array *this, unsigned int index, void *item) {
    unsigned int length = this->length + 1;
    update_capacity(this, length);
    this->length = length;
    void **items = this->items;
    for (unsigned int i = length - 1; i > index; i--) {
        items[i] = items[i - 1];
    }
    items[index] = item;
}

void array_insert_sort(Array *this, int (*compare)(void *, void *), void *item) {
    unsigned int len = this->length;
    void **items = this->items;
    for (unsigned int i = 0; i < len; i++) {
        if (compare(item, items[i]) <= 0) {
            array_insert(this, i, item);
            return;
        }
    }
    array_push(this, item);
}

void *array_find(Array *this, bool(find)(void *, void *), void *has) {
    unsigned int len = this->length;
    void **items = this->items;
    for (unsigned int i = 0; i < len; i++) {
        if (find(items[i], has)) {
            return items[i];
        }
    }
    return NULL;
}

void *array_get(Array *this, unsigned int index) {
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
    int len = this->length;
    void **items = this->items;
    for (int i = 0; i < len; i++) {
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

void array_remove_index(Array *this, unsigned int index) {
    this->length--;
    int len = this->length;
    void **items = this->items;
    for (int i = index; i < len; i++) {
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

unsigned int array_size(Array *this) {
    return this->length;
}

void release_array(Array *this) {
    free(this->items);
}

void delete_array(Array *this) {
    release_array(this);
    free(this);
}
