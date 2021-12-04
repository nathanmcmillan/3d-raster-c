/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "vec.h"

Vec *new_vec(float x, float y) {
    Vec *v = Malloc(sizeof(Vec));
    v->x = x;
    v->y = y;
    return v;
}

Vec *vec_copy(Vec *with) {
    Vec *v = Malloc(sizeof(Vec));
    v->x = with->x;
    v->y = with->y;
    return v;
}

Vec vec_of(Vec *with) {
    return (Vec){with->x, with->y};
}

Vec vec_normal(Vec *a, Vec *b) {
    float x = a->y - b->y;
    float y = -(a->x - b->x);
    float m = sqrtf(x * x + y * y);
    return (Vec){x / m, y / m};
}

bool line_intersect(Vec *va, Vec *vb, Vec *vc, Vec *vd) {
    float a1 = vb->y - va->y;
    float b1 = va->x - vb->x;
    float c1 = vb->x * va->y - va->x * vb->y;
    float r3 = a1 * vc->x + b1 * vc->y + c1;
    float r4 = a1 * vd->x + b1 * vd->y + c1;
    if (FLOAT_NOT_ZERO(r3) and FLOAT_NOT_ZERO(r4) and r3 * r4 >= 0) {
        return false;
    }
    float a2 = vd->y - vc->y;
    float b2 = vc->x - vd->x;
    float c2 = vd->x * vc->y - vc->x * vd->y;
    float r1 = a2 * va->x + b2 * va->y + c2;
    float r2 = a2 * vb->x + b2 * vb->y + c2;
    if (FLOAT_NOT_ZERO(r1) and FLOAT_NOT_ZERO(r2) and r1 * r2 >= 0) {
        return false;
    }
    float denominator = a1 * b2 - a2 * b1;
    return FLOAT_NOT_ZERO(denominator);
}

MaybeVec line_intersect_at(Vec *va, Vec *vb, Vec *vc, Vec *vd) {
    float a1 = vb->y - va->y;
    float b1 = va->x - vb->x;
    float c1 = vb->x * va->y - va->x * vb->y;
    float r3 = a1 * vc->x + b1 * vc->y + c1;
    float r4 = a1 * vd->x + b1 * vd->y + c1;
    if (FLOAT_NOT_ZERO(r3) and FLOAT_NOT_ZERO(r4) and r3 * r4 >= 0) {
        return (MaybeVec){{0, 0}, false};
    }
    float a2 = vd->y - vc->y;
    float b2 = vc->x - vd->x;
    float c2 = vd->x * vc->y - vc->x * vd->y;
    float r1 = a2 * va->x + b2 * va->y + c2;
    float r2 = a2 * vb->x + b2 * vb->y + c2;
    if (FLOAT_NOT_ZERO(r1) and FLOAT_NOT_ZERO(r2) and r1 * r2 >= 0) {
        return (MaybeVec){{0, 0}, false};
    }
    float denominator = a1 * b2 - a2 * b1;
    if (FLOAT_ZERO(denominator)) {
        return (MaybeVec){{0, 0}, false};
    }
    float offset;
    if (denominator < 0) {
        offset = -denominator * 0.5f;
    } else {
        offset = denominator * 0.5f;
    }
    float number = b1 * c2 - b2 * c1;
    float x;
    float y;
    if (number < 0) {
        x = (number - offset) / denominator;
    } else {
        x = (number + offset) / denominator;
    }
    number = a2 * c1 - a1 * c2;
    if (number < 0) {
        y = (number - offset) / denominator;
    } else {
        y = (number + offset) / denominator;
    }
    return (MaybeVec){{x, y}, true};
}

// export function lineIntersectAt(out, ax, ay, bx, by, cx, cy, dx, dy) {
//   const a1 = by - ay
//   const a2 = dy - cy
//   const a3 = bx - ax
//   const a4 = ay - cy
//   const a5 = ax - cx
//   const a6 = dx - cx
//   const div = a2 * a3 - a6 * a1
//   const uA = (a6 * a4 - a2 * a5) / div
//   if (uA < 0.0 || uA > 1.0) return false
//   const uB = (a3 * a4 - a1 * a5) / div
//   if (uB < 0.0 || uB > 1.0) return false
//   out[0] = ax + uA * a3
//   out[1] = ay + uA * a1
//   return true
// }

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
