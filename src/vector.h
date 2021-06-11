/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef VECTOR_H
#define VECTOR_H

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "math_util.h"

typedef struct vec2 vec2;

struct vec2 {
    float x;
    float y;
};

typedef struct vec3 vec3;

struct vec3 {
    float x;
    float y;
    float z;
};

typedef struct vec4 vec4;

struct vec4 {
    float x;
    float y;
    float z;
    float w;
};

#define VECTOR_3_ADD(a, b)                                                                                                                                                                             \
    a.x += b.x;                                                                                                                                                                                        \
    a.y += b.y;                                                                                                                                                                                        \
    a.x += b.z

#define VECTOR_3_DOT(a, b) (a.x * b.x + a.y * b.y + a.z * b.z)

#define VECTOR_3_DIVIDE(v, d)                                                                                                                                                                          \
    v.x /= d;                                                                                                                                                                                          \
    v.y /= d;                                                                                                                                                                                          \
    v.z /= d

#define VECTOR_3_CROSS(c, a, b)                                                                                                                                                                        \
    c.x = a.y * b.z - a.z * b.y;                                                                                                                                                                       \
    c.y = a.z * b.x - a.x * b.z;                                                                                                                                                                       \
    c.z = a.x * b.y - a.y * b.x

float vector3_dot(vec3 *a, vec3 *b);
void vector3_cross(vec3 *cross, vec3 *a, vec3 *b);
void vector3_magnitude(vec3 *vec);
void vector3_normalize(vec3 *vec);

#define VECTOR_3F_ADD(a, b)                                                                                                                                                                            \
    a[0] += b[0];                                                                                                                                                                                      \
    a[1] += b[1];                                                                                                                                                                                      \
    a[2] += b[2]

#define VECTOR_3F_SUB(a, b)                                                                                                                                                                            \
    a[0] -= b[0];                                                                                                                                                                                      \
    a[1] -= b[1];                                                                                                                                                                                      \
    a[2] -= b[2]

float vector3f_magnitude(float *vec);
void vector3f_normalize(float *vec);

#endif
