/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef UINT_TABLE_H
#define UINT_TABLE_H

#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>

#include "mem.h"

typedef struct UIntTableItem UIntTableItem;
typedef struct UIntTable UIntTable;
typedef struct UIntTablePair UIntTablePair;
typedef struct UIntTableIter UIntTableIter;

struct UIntTableItem {
    unsigned long hash;
    unsigned int key;
    void *value;
    UIntTableItem *next;
};

struct UIntTable {
    unsigned int size;
    unsigned int bins;
    UIntTableItem **items;
};

UIntTable *new_uint_table();

void uint_table_put(UIntTable *this, unsigned int key, void *value);
void *uint_table_get(UIntTable *this, unsigned int key);
bool uint_table_has(UIntTable *this, unsigned int key);

void *uint_table_remove(UIntTable *this, unsigned int key);
void uint_table_clear(UIntTable *this);

bool uint_table_is_empty(UIntTable *this);
bool uint_table_not_empty(UIntTable *this);
unsigned int uint_table_size(UIntTable *this);

void uint_table_release(UIntTable *this);
void uint_table_delete(UIntTable *this);

struct UIntTablePair {
    unsigned int key;
    void *value;
};

struct UIntTableIter {
    UIntTable *pointer;
    unsigned int bin;
    UIntTableItem *item;
};

UIntTableIter new_uint_table_iterator(UIntTable *this);
bool uint_table_iterator_has_next(UIntTableIter *iter);
UIntTablePair uint_table_iterator_next(UIntTableIter *iter);

#endif
