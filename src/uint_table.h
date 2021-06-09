/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef UINT_TABLE_H
#define UINT_TABLE_H

#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>

#include "mem.h"

typedef struct uint_table_item uint_table_item;

struct uint_table_item {
    unsigned long hash;
    unsigned int key;
    void *value;
    uint_table_item *next;
};

typedef struct uint_table uint_table;

struct uint_table {
    unsigned int size;
    unsigned int bins;
    uint_table_item **items;
};

uint_table *create_uint_table();

void uint_table_put(uint_table *this, unsigned int key, void *value);
void *uint_table_get(uint_table *this, unsigned int key);
bool uint_table_has(uint_table *this, unsigned int key);

void *uint_table_remove(uint_table *this, unsigned int key);
void uint_table_clear(uint_table *this);

bool uint_table_is_empty(uint_table *this);
bool uint_table_not_empty(uint_table *this);
unsigned int uint_table_size(uint_table *this);

void release_uint_table(uint_table *this);
void delete_uint_table(uint_table *this);

typedef struct uint_table_pair uint_table_pair;

struct uint_table_pair {
    unsigned int key;
    void *value;
};

typedef struct uint_table_iterator uint_table_iterator;

struct uint_table_iterator {
    uint_table *pointer;
    unsigned int bin;
    uint_table_item *item;
};

uint_table_iterator create_uint_table_iterator(uint_table *this);
bool uint_table_iterator_has_next(uint_table_iterator *iter);
uint_table_pair uint_table_iterator_next(uint_table_iterator *iter);

#endif
