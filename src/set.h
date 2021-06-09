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

typedef struct set_item set_item;

struct set_item {
    unsigned long hash;
    void *key;
    set_item *next;
};

typedef struct set set;

struct set {
    bool (*equals_fn)(void *, void *);
    unsigned long (*hashcode_fn)(void *);
    unsigned int size;
    unsigned int bins;
    set_item **items;
};

bool set_string_equal(void *a, void *b);
unsigned long set_string_hashcode(void *key);

bool set_address_equal(void *a, void *b);
unsigned long set_address_hashcode(void *key);

set *new_set(bool (*equals_fn)(void *, void *), unsigned long (*hashcode_fn)(void *));

void set_add(set *this, void *key);
bool set_has(set *this, void *key);

void set_remove(set *this, void *key);
void set_clear(set *this);

bool set_is_empty(set *this);
bool set_not_empty(set *this);
unsigned int set_size(set *this);

void set_release(set *this);
void set_delete(set *this);

typedef struct set_iterator set_iterator;

struct set_iterator {
    set *pointer;
    unsigned int bin;
    set_item *item;
};

set_iterator new_set_iterator(set *this);
bool set_iterator_has_next(set_iterator *iter);
void *set_iterator_next(set_iterator *iter);

#endif
