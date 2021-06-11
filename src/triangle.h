/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <stdbool.h>

#include "mem.h"
#include "vec.h"

typedef struct Triangle Triangle;

struct Triangle {
    float height;
    int texture;
    Vec va;
    Vec vb;
    Vec vc;
    float u1;
    float v1;
    float u2;
    float v2;
    float u3;
    float v3;
    float normal;
};

Triangle *new_triangle(float height, int texture, Vec va, Vec vb, Vec vc, bool floor, float scale);

#endif
