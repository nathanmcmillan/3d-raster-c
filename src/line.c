/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "sector.h"

Line *new_line(int id, Vec *a, Vec *b, int bottom, int middle, int top) {
    Line *line = Calloc(1, sizeof(Line));
    line->id = id;
    line->a = a;
    line->b = b;
    line->normal = vec_normal(a, b);
    line->side_front.top = top;
    line->side_front.middle = middle;
    line->side_front.bottom = bottom;
    return line;
}
