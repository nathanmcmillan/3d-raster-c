/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "sector.h"

unsigned int sector_unique_id = 0;

Sector *sector_init(Vec **vecs, int vec_count, Line **lines, int line_count, float bottom, float floor, float ceiling, float top, int floor_texture, int ceiling_texture) {
    Sector *s = safe_calloc(1, sizeof(Sector));
    s->id = sector_unique_id++;
    s->vecs = vecs;
    s->vec_count = vec_count;
    s->lines = lines;
    s->line_count = line_count;
    s->bottom = bottom;
    s->floor = floor;
    s->ceiling = ceiling;
    s->top = top;
    s->floor_texture = floor_texture;
    s->ceiling_texture = ceiling_texture;
    return s;
}

bool sector_contains(Sector *this, float x, float y) {
    bool odd = false;
    Vec **vecs = this->vecs;
    int count = this->vec_count;
    int j = count - 1;
    for (int i = 0; i < count; i++) {
        Vec *vi = vecs[i];
        Vec *vj = vecs[j];

        if ((vi->y > y) != (vj->y > y)) {
            float value = (vj->x - vi->x) * (y - vi->y) / (vj->y - vi->y) + vi->x;
            if (x < value) {
                odd = !odd;
            }
        }

        j = i;
    }
    return odd;
}

Sector *sector_find(Sector *this, float x, float y) {
    Sector **inside = this->inside;
    int count = this->inside_count;
    for (int i = 0; i < count; i++) {
        Sector *s = inside[i];
        if (sector_contains(s, x, y)) {
            return sector_find(s, x, y);
        }
    }
    return this;
}

bool sector_has_floor(Sector *this) {
    return this->floor_texture != SECTOR_NO_SURFACE;
}

bool sector_has_ceiling(Sector *this) {
    return this->ceiling_texture != SECTOR_NO_SURFACE;
}
