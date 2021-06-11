/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef VEC_H
#define VEC_H

#include <stdbool.h>

#include "math_util.h"
#include "mem.h"
#include "pie.h"

typedef struct Vec Vec;

struct Vec {
    float x;
    float y;
};

typedef struct VecOk VecOk;

struct VecOk {
    Vec v;
    bool ok;
};

Vec *vec_init(float x, float y);
Vec *vec_copy(Vec *with);
Vec vec_of(Vec *with);
bool vec_intersect(Vec *va, Vec *vb, Vec *vc, Vec *vd);

#endif
