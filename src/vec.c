/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "vec.h"

vec *vec_init(float x, float y) {
    vec *v = safe_malloc(sizeof(vec));
    v->x = x;
    v->y = y;
    return v;
}

vec *vec_copy(vec *with) {
    vec *v = safe_malloc(sizeof(vec));
    v->x = with->x;
    v->y = with->y;
    return v;
}

vec vec_of(vec *with) {
    return (vec){with->x, with->y};
}

bool vec_intersect(vec *va, vec *vb, vec *vc, vec *vd) {

    float a1 = vb->y - va->y;
    float b1 = va->x - vb->x;
    float c1 = (vb->x * va->y) - (va->x * vb->y);

    float r3 = (a1 * vc->x) + (b1 * vc->y) + c1;
    float r4 = (a1 * vd->x) + (b1 * vd->y) + c1;

    if (FLOAT_NOT_ZERO(r3) && FLOAT_NOT_ZERO(r4) && r3 * r4 >= 0) {
        return false;
    }

    float a2 = vd->y - vc->y;
    float b2 = vc->x - vd->x;
    float c2 = (vd->x * vc->y) - (vc->x * vd->y);

    float r1 = (a2 * va->x) + (b2 * va->y) + c2;
    float r2 = (a2 * vb->x) + (b2 * vb->y) + c2;

    if (FLOAT_NOT_ZERO(r1) && FLOAT_NOT_ZERO(r2) && r1 * r2 >= 0) {
        return false;
    }

    float denominator = (a1 * b2) - (a2 * b1);

    if (FLOAT_ZERO(denominator)) {
        return false;
    }

    return true;
}
