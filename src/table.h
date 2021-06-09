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

typedef struct TableItem TableItem;
typedef struct Table Table;
typedef struct TablePair TablePair;
typedef struct TableIter TableIter;

struct TableItem {
    unsigned long hash;
    void *key;
    void *value;
    TableItem *next;
};

struct Table {
    bool (*equals_fn)(void *, void *);
    unsigned long (*hashcode_fn)(void *);
    unsigned int size;
    unsigned int bins;
    TableItem **items;
};

bool table_string_equal(void *a, void *b);
unsigned long table_string_hashcode(void *key);

bool table_address_equal(void *a, void *b);
unsigned long table_address_hashcode(void *key);

Table *create_table(bool (*equals_fn)(void *, void *), unsigned long (*hashcode_fn)(void *));

void table_put(Table *this, void *key, void *value);
void *table_get(Table *this, void *key);
bool table_has(Table *this, void *key);

void *table_remove(Table *this, void *key);
void table_clear(Table *this);

bool table_is_empty(Table *this);
bool table_not_empty(Table *this);
unsigned int table_size(Table *this);

void table_release(Table *this);
void table_delete(Table *this);

struct TablePair {
    void *key;
    void *value;
};

struct TableIter {
    Table *pointer;
    unsigned int bin;
    TableItem *item;
};

TableIter create_table_iterator(Table *this);
bool table_iterator_has_next(TableIter *iter);
TablePair table_iterator_next(TableIter *iter);

#endif
