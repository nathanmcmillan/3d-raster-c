/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "array.h"

bool find_address(void *item, void *has) {
    return item == has;
}

void array_init_with_capacity(array *this, unsigned int length, unsigned int capacity) {
    if (capacity == 0) {
        this->items = NULL;
    } else {
        this->items = safe_calloc(capacity, sizeof(void *));
    }
    this->length = length;
    this->capacity = capacity;
}

void array_init(array *this, unsigned int length) {
    array_init_with_capacity(this, length, length);
}

array *create_array_with_capacity(unsigned int length, unsigned int capacity) {
    array *this = safe_malloc(sizeof(array));
    array_init_with_capacity(this, length, capacity);
    return this;
}

array *create_array(unsigned int length) {
    return create_array_with_capacity(length, length);
}

array *create_array_with_items(unsigned int length, unsigned int capacity, void **items) {
    array *this = safe_malloc(sizeof(array));
    this->items = items;
    this->length = length;
    this->capacity = capacity;
    return this;
}

void **array_copy_items(array *this) {
    size_t size = this->length * sizeof(void *);
    void **copy = safe_malloc(size);
    memcpy(copy, this->items, size);
    return copy;
}

array *create_array_copy(array *from) {
    array *this = safe_malloc(sizeof(array));
    this->items = array_copy_items(from);
    this->length = from->length;
    this->capacity = from->length;
    return this;
}

static void update_capacity(array *this, unsigned int length) {
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

void array_push(array *this, void *item) {
    unsigned int length = this->length + 1;
    update_capacity(this, length);
    this->length = length;
    this->items[length - 1] = item;
}

void array_insert(array *this, unsigned int index, void *item) {
    unsigned int length = this->length + 1;
    update_capacity(this, length);
    this->length = length;
    void **items = this->items;
    for (unsigned int i = length - 1; i > index; i--) {
        items[i] = items[i - 1];
    }
    items[index] = item;
}

void array_insert_sort(array *this, int (*compare)(void *, void *), void *item) {
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

void *array_find(array *this, bool(find)(void *, void *), void *has) {
    unsigned int len = this->length;
    void **items = this->items;
    for (unsigned int i = 0; i < len; i++) {
        if (find(items[i], has)) {
            return items[i];
        }
    }
    return NULL;
}

void *array_get(array *this, unsigned int index) {
    if (index >= this->length) {
        return NULL;
    }
    return this->items[index];
}

void *array_pop(array *this) {
    if (this->length == 0) {
        return NULL;
    }
    this->length--;
    return this->items[this->length];
}

void array_remove(array *this, void *item) {
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

void array_remove_index(array *this, unsigned int index) {
    this->length--;
    int len = this->length;
    void **items = this->items;
    for (int i = index; i < len; i++) {
        items[i] = items[i + 1];
    }
    items[len] = NULL;
}

void array_clear(array *this) {
    this->length = 0;
}

bool array_is_empty(array *this) {
    return this->length == 0;
}

bool array_not_empty(array *this) {
    return this->length != 0;
}

unsigned int array_size(array *this) {
    return this->length;
}

void release_array(array *this) {
    free(this->items);
}

void delete_array(array *this) {
    release_array(this);
    free(this);
}
