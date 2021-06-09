/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef TABLE_H
#define TABLE_H

#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>

#include "mem.h"
#include "pie.h"

typedef struct table_item table_item;

struct table_item {
    unsigned long hash;
    void *key;
    void *value;
    table_item *next;
};

typedef struct table table;

struct table {
    bool (*equals_fn)(void *, void *);
    unsigned long (*hashcode_fn)(void *);
    unsigned int size;
    unsigned int bins;
    table_item **items;
};

bool table_string_equal(void *a, void *b);
unsigned long table_string_hashcode(void *key);

bool table_address_equal(void *a, void *b);
unsigned long table_address_hashcode(void *key);

table *create_table(bool (*equals_fn)(void *, void *), unsigned long (*hashcode_fn)(void *));

void table_put(table *this, void *key, void *value);
void *table_get(table *this, void *key);
bool table_has(table *this, void *key);

void *table_remove(table *this, void *key);
void table_clear(table *this);

bool table_is_empty(table *this);
bool table_not_empty(table *this);
unsigned int table_size(table *this);

void release_table(table *this);
void delete_table(table *this);

typedef struct table_pair table_pair;

struct table_pair {
    void *key;
    void *value;
};

typedef struct table_iterator table_iterator;

struct table_iterator {
    table *pointer;
    unsigned int bin;
    table_item *item;
};

table_iterator create_table_iterator(table *this);
bool table_iterator_has_next(table_iterator *iter);
table_pair table_iterator_next(table_iterator *iter);

#endif
