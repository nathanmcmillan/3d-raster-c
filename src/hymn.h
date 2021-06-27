/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef HYMN_H
#define HYMN_H

#include "fileio.h"
#include "log.h"
#include "mem.h"
#include "pie.h"
#include "string_util.h"
#include "table.h"

#define HYMN_DEBUG_TRACE
#define HYMN_DEBUG_STACK

#define HYMN_STACK_MAX 256

typedef struct Hymn Hymn;

struct Hymn {
    Table *scripts;
};

Hymn *new_hymn();

char *hymn_eval(Hymn *this, char *content);
char *hymn_read(Hymn *this, char *file);
char *hymn_repl(Hymn *this);

void hymn_add_func(Hymn *this, char *name, char *(*func)(Hymn *));
void hymn_add_pointer(Hymn *this, char *name, void *pointer);

char *hymn_call(Hymn *this, char *name);

void *hymn_pointer(Hymn *this, i32 index);
i32 hymn_i32(Hymn *this, i32 index);
u32 hymn_u32(Hymn *this, i32 index);
i64 hymn_i64(Hymn *this, i32 index);
u64 hymn_u64(Hymn *this, i32 index);
f32 hymn_f32(Hymn *this, i32 index);
f64 hymn_f64(Hymn *this, i32 index);

void hymn_delete(Hymn *this);

#endif