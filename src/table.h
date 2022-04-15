/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef TABLE_H
#define TABLE_H

#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>

#include "mem.h"
#include "super.h"

typedef struct TableItem TableItem;
typedef struct Table Table;
typedef struct TablePair TablePair;
typedef struct TableIter TableIter;

struct TableItem {
    usize hash;
    void *key;
    void *value;
    TableItem *next;
};

struct Table {
    bool (*equals_fn)(void *, void *);
    usize (*hashcode_fn)(void *);
    unsigned int size;
    unsigned int bins;
    TableItem **items;
};

bool table_string_equal(void *a, void *b);
usize table_string_hashcode(void *key);

bool table_address_equal(void *a, void *b);
usize table_address_hashcode(void *key);

Table *new_table(bool (*equals_fn)(void *, void *), usize (*hashcode_fn)(void *));
Table *new_string_table();
Table *new_pointer_table();

void TablePut(Table *this, void *key, void *value);
void *TableGet(Table *this, void *key);
bool TableHas(Table *this, void *key);

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

TableIter NewTableIterator(Table *this);
bool TableIteratorHasNext(TableIter *iter);
TablePair TableIteratorNext(TableIter *iter);

#endif
