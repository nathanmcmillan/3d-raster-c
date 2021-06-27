/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef VEC_H
#define VEC_H

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "math_util.h"
#include "mem.h"
#include "pie.h"

#define VECTOR_3_ADD(a, b) \
    a.x += b.x;            \
    a.y += b.y;            \
    a.x += b.z

#define VECTOR_3_DOT(a, b) (a.x * b.x + a.y * b.y + a.z * b.z)

#define VECTOR_3_DIVIDE(v, d) \
    v.x /= d;                 \
    v.y /= d;                 \
    v.z /= d

#define VECTOR_3_CROSS(c, a, b)  \
    c.x = a.y * b.z - a.z * b.y; \
    c.y = a.z * b.x - a.x * b.z; \
    c.z = a.x * b.y - a.y * b.x

#define VECTOR_3F_ADD(a, b) \
    a[0] += b[0];           \
    a[1] += b[1];           \
    a[2] += b[2]

#define VECTOR_3F_SUB(a, b) \
    a[0] -= b[0];           \
    a[1] -= b[1];           \
    a[2] -= b[2]

typedef struct Vec Vec;
typedef struct MaybeVec MaybeVec;

typedef struct Vec2 Vec2;
typedef struct Vec3 Vec3;
typedef struct Vec4 Vec4;

struct Vec {
    float x;
    float y;
};

struct MaybeVec {
    Vec v;
    bool ok;
};

Vec *new_vec(float x, float y);
Vec *vec_copy(Vec *with);
Vec vec_of(Vec *with);
bool vec_intersect(Vec *va, Vec *vb, Vec *vc, Vec *vd);

struct Vec2 {
    float x;
    float y;
};

struct Vec3 {
    float x;
    float y;
    float z;
};

struct Vec4 {
    float x;
    float y;
    float z;
    float w;
};

float vector3_dot(Vec3 *a, Vec3 *b);
void vector3_cross(Vec3 *cross, Vec3 *a, Vec3 *b);
void vector3_magnitude(Vec3 *vec);
void vector3_normalize(Vec3 *vec);

float vector3f_magnitude(float *vec);
void vector3f_normalize(float *vec);

#endif
