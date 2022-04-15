/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef SET_H
#define SET_H

#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>

#include "mem.h"
#include "super.h"

typedef struct SetItem SetItem;
typedef struct Set Set;
typedef struct SetIterator SetIterator;

struct SetItem {
    usize hash;
    void *key;
    SetItem *next;
};

struct Set {
    bool (*equals_fn)(void *, void *);
    usize (*hashcode_fn)(void *);
    unsigned int size;
    unsigned int bins;
    SetItem **items;
};

bool set_string_equal(void *a, void *b);
usize set_string_hashcode(void *key);

bool set_address_equal(void *a, void *b);
usize set_address_hashcode(void *key);

Set *new_set(bool (*equals_fn)(void *, void *), usize (*hashcode_fn)(void *));

Set *NewAddressSet();

void SetAdd(Set *this, void *key);
bool SetHas(Set *this, void *key);

void set_remove(Set *this, void *key);
void SetClear(Set *this);

bool set_is_empty(Set *this);
bool set_not_empty(Set *this);
unsigned int set_size(Set *this);

void set_release(Set *this);
void SetFree(Set *this);

struct SetIterator {
    Set *pointer;
    unsigned int bin;
    SetItem *item;
};

SetIterator NewSetIterator(Set *this);
bool SetIteratorHasNext(SetIterator *iter);
void *SetIteratorNext(SetIterator *iter);

#endif
