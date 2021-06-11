/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "sector.h"

wall *wall_init(line *ld, vec va, vec vb, int texture) {
    wall *w = safe_calloc(1, sizeof(wall));
    w->ld = ld;
    w->va = va;
    w->vb = vb;
    w->texture = texture;
    return w;
}

void wall_set(wall *this, float floor, float ceiling, float u, float v, float s, float t) {
    this->floor = floor;
    this->ceiling = ceiling;
    this->u = u;
    this->v = v;
    this->s = s;
    this->t = t;
}
