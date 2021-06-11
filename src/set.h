/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef SET_H
#define SET_H

#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>

#include "mem.h"
#include "pie.h"
#include "slice.h"

typedef struct SetItem SetItem;
typedef struct Set Set;
typedef struct SetIterator SetIterator;

struct SetItem {
    unsigned long hash;
    void *key;
    SetItem *next;
};

struct Set {
    bool (*equals_fn)(void *, void *);
    unsigned long (*hashcode_fn)(void *);
    unsigned int size;
    unsigned int bins;
    SetItem **items;
};

bool set_string_equal(void *a, void *b);
unsigned long set_string_hashcode(void *key);

bool set_address_equal(void *a, void *b);
unsigned long set_address_hashcode(void *key);

Set *new_set(bool (*equals_fn)(void *, void *), unsigned long (*hashcode_fn)(void *));

void set_add(Set *this, void *key);
bool set_has(Set *this, void *key);

void set_remove(Set *this, void *key);
void set_clear(Set *this);

bool set_is_empty(Set *this);
bool set_not_empty(Set *this);
unsigned int set_size(Set *this);

void set_release(Set *this);
void set_delete(Set *this);

struct SetIterator {
    Set *pointer;
    unsigned int bin;
    SetItem *item;
};

SetIterator new_set_iterator(Set *this);
bool set_iterator_has_next(SetIterator *iter);
void *set_iterator_next(SetIterator *iter);

#endif
