/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef VEC_H
#define VEC_H

#include <stdbool.h>

#include "math_util.h"
#include "mem.h"

typedef struct vec vec;

struct vec {
    float x;
    float y;
};

typedef struct vec_ok vec_ok;

struct vec_ok {
    vec v;
    bool ok;
};

vec *vec_init(float x, float y);
vec *vec_copy(vec *with);
vec vec_of(vec *with);
bool vec_intersect(vec *va, vec *vb, vec *vc, vec *vd);

#endif
