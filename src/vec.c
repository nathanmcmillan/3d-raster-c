/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "vec.h"

Vec *vec_init(float x, float y) {
    Vec *v = safe_malloc(sizeof(Vec));
    v->x = x;
    v->y = y;
    return v;
}

Vec *vec_copy(Vec *with) {
    Vec *v = safe_malloc(sizeof(Vec));
    v->x = with->x;
    v->y = with->y;
    return v;
}

Vec vec_of(Vec *with) {
    return (Vec){with->x, with->y};
}

bool vec_intersect(Vec *va, Vec *vb, Vec *vc, Vec *vd) {

    float a1 = vb->y - va->y;
    float b1 = va->x - vb->x;
    float c1 = (vb->x * va->y) - (va->x * vb->y);

    float r3 = (a1 * vc->x) + (b1 * vc->y) + c1;
    float r4 = (a1 * vd->x) + (b1 * vd->y) + c1;

    if (FLOAT_NOT_ZERO(r3) and FLOAT_NOT_ZERO(r4) and r3 * r4 >= 0) {
        return false;
    }

    float a2 = vd->y - vc->y;
    float b2 = vc->x - vd->x;
    float c2 = (vd->x * vc->y) - (vc->x * vd->y);

    float r1 = (a2 * va->x) + (b2 * va->y) + c2;
    float r2 = (a2 * vb->x) + (b2 * vb->y) + c2;

    if (FLOAT_NOT_ZERO(r1) and FLOAT_NOT_ZERO(r2) and r1 * r2 >= 0) {
        return false;
    }

    float denominator = (a1 * b2) - (a2 * b1);

    if (FLOAT_ZERO(denominator)) {
        return false;
    }

    return true;
}
