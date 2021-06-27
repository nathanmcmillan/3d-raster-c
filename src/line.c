/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "sector.h"

Line *new_line(Vec *a, Vec *b, int low, int mid, int top) {
    Line *line = safe_calloc(1, sizeof(Line));
    line->a = a;
    line->b = b;
    if (low >= 0) {
        line->bottom = new_wall(a, b, low);
    }
    if (mid >= 0) {
        line->middle = new_wall(a, b, mid);
    }
    if (top >= 0) {
        line->top = new_wall(a, b, top);
    }
    return line;
}

void line_set_sectors(Line *this, Sector *plus, Sector *minus) {
    this->plus = plus;
    this->minus = minus;
    float x = this->a->y - this->b->y;
    float y = -(this->a->x - this->b->x);
    float m = sqrtf(x * x + y * y);
    this->normal = (Vec){x / m, y / m};
}

MaybeVec line_intersect(Line *this, Line *with) {
    float a1 = this->b->y - this->a->y;
    float b1 = this->a->x - this->b->x;
    float c1 = (this->b->x * this->a->y) - (this->a->x * this->b->y);

    float r3 = (a1 * with->a->x) + (b1 * with->a->y) + c1;
    float r4 = (a1 * with->b->x) + (b1 * with->b->y) + c1;

    if (FLOAT_NOT_ZERO(r3) and FLOAT_NOT_ZERO(r4) and r3 * r4 >= 0) {
        return (MaybeVec){{0, 0}, false};
    }

    float a2 = with->b->y - with->a->y;
    float b2 = with->a->x - with->b->x;
    float c2 = (with->b->x * with->a->y) - (with->a->x * with->b->y);

    float r1 = (a2 * this->a->x) + (b2 * this->a->y) + c2;
    float r2 = (a2 * this->b->x) + (b2 * this->b->y) + c2;

    if (FLOAT_NOT_ZERO(r1) and FLOAT_NOT_ZERO(r2) and r1 * r2 >= 0) {
        return (MaybeVec){{0, 0}, false};
    }

    float denominator = (a1 * b2) - (a2 * b1);

    if (FLOAT_ZERO(denominator)) {
        return (MaybeVec){{0, 0}, false};
    }

    float offset;

    if (denominator < 0) {
        offset = -denominator * 0.5f;
    } else {
        offset = denominator * 0.5f;
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

    return (MaybeVec){{x, y}, true};
}
