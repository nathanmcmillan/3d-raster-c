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

typedef void *Slice;
typedef struct SliceHead SliceHead;

PACK(struct SliceHead {
    usize length;
    usize capacity;
    void **Slice;
});

Slice slice_init(const usize member_size, const usize length, const usize capacity);
Slice slice_simple_init(const usize member_size, const usize length);
Slice array_to_slice(void *const array, const usize member_size, const usize length);
void slice_free(const Slice a);
usize slice_len_size(const Slice a);
int slice_len(const Slice a);
usize slice_cap_size(const Slice a);
int slice_cap(const Slice a);
SliceHead *slice_resize(const Slice head, const usize member_size, const usize length);
Slice slice_expand(const Slice a, const Slice b);
Slice slice_push(const Slice a, void *const b);
Slice slice_push_int(const Slice a, const int b);
Slice slice_push_float(const Slice a, const float b);
void *slice_pop(const Slice a);
int slice_pop_int(const Slice a);
float slice_pop_float(const Slice a);

#endif
