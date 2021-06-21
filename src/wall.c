/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "sector.h"

Wall *new_wall(Vec *a, Vec *b, int texture) {
    Wall *w = safe_calloc(1, sizeof(Wall));
    w->a = a;
    w->b = b;
    w->texture = texture;
    return w;
}

void wall_set(Wall *this, float floor, float ceiling, float u, float v, float s, float t) {
    this->floor = floor;
    this->ceiling = ceiling;
    this->u = u;
    this->v = v;
    this->s = s;
    this->t = t;
}
