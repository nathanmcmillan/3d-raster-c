/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "vector.h"

float vector3_dot(Vec3 *a, Vec3 *b) {
    return a->x * b->x + a->y * b->y + a->z * b->z;
}

void vector3_cross(Vec3 *cross, Vec3 *a, Vec3 *b) {
    cross->x = a->y * b->z - a->z * b->y;
    cross->y = a->z * b->x - a->x * b->z;
    cross->z = a->x * b->y - a->y * b->x;
}

void vector3_normalize(Vec3 *vec) {
    float magnitude = sqrtf(vec->x * vec->x + vec->y * vec->y + vec->z * vec->z);
    float multiple = 1.0f / magnitude;
    vec->x *= multiple;
    vec->y *= multiple;
    vec->z *= multiple;
}

float vector3f_magnitude(float *vec) {
    return sqrtf(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
}

void vector3f_normalize(float *vec) {
    float multiple = 1.0f / vector3f_magnitude(vec);
    vec[0] *= multiple;
    vec[1] *= multiple;
    vec[2] *= multiple;
}
