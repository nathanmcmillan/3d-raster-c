/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef SLICE_H
#define SLICE_H

#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mem.h"
#include "pie.h"

typedef void *slice;
typedef struct slice_head slice_head;

PACK(struct slice_head {
    usize length;
    usize capacity;
    void **slice;
});

slice slice_init(const usize member_size, const usize length, const usize capacity);
slice slice_simple_init(const usize member_size, const usize length);
slice array_to_slice(void *const array, const usize member_size, const usize length);
void slice_free(const slice a);
usize slice_len_size(const slice a);
int slice_len(const slice a);
usize slice_cap_size(const slice a);
int slice_cap(const slice a);
slice_head *slice_resize(const slice head, const usize member_size, const usize length);
slice slice_expand(const slice a, const slice b);
slice slice_push(const slice a, void *const b);
slice slice_push_int(const slice a, const int b);
slice slice_push_float(const slice a, const float b);
void *slice_pop(const slice a);
int slice_pop_int(const slice a);
float slice_pop_float(const slice a);

#endif
