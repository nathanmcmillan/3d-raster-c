/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "sector.h"

line *line_init(vec va, vec vb, int low, int mid, int top) {
    line *ld = safe_calloc(1, sizeof(line));
    ld->va = va;
    ld->vb = vb;
    if (low >= 0) {
        ld->bottom = wall_init(ld, va, vb, low);
    }
    if (mid >= 0) {
        ld->middle = wall_init(ld, va, vb, mid);
    }
    if (top >= 0) {
        ld->top = wall_init(ld, va, vb, top);
    }
    return ld;
}

void line_set_sectors(line *this, sector *plus, sector *minus) {
    this->plus = plus;
    this->minus = minus;
    float x = this->va.y - this->vb.y;
    float y = -(this->va.x - this->vb.x);
    float m = sqrt(x * x + y * y);
    this->normal = (vec){x / m, y / m};
}

vec_ok line_intersect(line *this, line *with) {
    float a1 = this->vb.y - this->va.y;
    float b1 = this->va.x - this->vb.x;
    float c1 = (this->vb.x * this->va.y) - (this->va.x * this->vb.y);

    float r3 = (a1 * with->va.x) + (b1 * with->va.y) + c1;
    float r4 = (a1 * with->vb.x) + (b1 * with->vb.y) + c1;

    if (FLOAT_NOT_ZERO(r3) && FLOAT_NOT_ZERO(r4) && r3 * r4 >= 0) {
        return (vec_ok){{0, 0}, false};
    }

    float a2 = with->vb.y - with->va.y;
    float b2 = with->va.x - with->vb.x;
    float c2 = (with->vb.x * with->va.y) - (with->va.x * with->vb.y);

    float r1 = (a2 * this->va.x) + (b2 * this->va.y) + c2;
    float r2 = (a2 * this->vb.x) + (b2 * this->vb.y) + c2;

    if (FLOAT_NOT_ZERO(r1) && FLOAT_NOT_ZERO(r2) && r1 * r2 >= 0) {
        return (vec_ok){{0, 0}, false};
    }

    float denominator = (a1 * b2) - (a2 * b1);

    if (FLOAT_ZERO(denominator)) {
        return (vec_ok){{0, 0}, false};
    }

    float offset;

    if (denominator < 0) {
        offset = -denominator * 0.5;
    } else {
        offset = denominator * 0.5;
    }

    float number = (b1 * c2) - (b2 * c1);

    float x;
    float y;

    if (number < 0) {
        x = (number - offset) / denominator;
    } else {
        x = (number + offset) / denominator;
    }

    number = (a2 * c1) - (a1 * c2);

    if (number < 0) {
        y = (number - offset) / denominator;
    } else {
        y = (number + offset) / denominator;
    }

    return (vec_ok){{x, y}, true};
}
