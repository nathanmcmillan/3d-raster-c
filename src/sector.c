/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "sector.h"

unsigned int sector_unique_id = 0;

Sector *new_sector(Vec **vecs, int vec_count, Line **lines, int line_count, float bottom, float floor, float ceiling, float top, int floor_paint, int ceiling_paint) {
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
    s->floor_paint = floor_paint;
    s->ceiling_paint = ceiling_paint;
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
    return this->floor_paint != SECTOR_NO_SURFACE;
}

bool sector_has_ceiling(Sector *this) {
    return this->ceiling_paint != SECTOR_NO_SURFACE;
}

void sector_inside_outside(Sector **sectors, int sector_count) {

    Array *sector_inside_lists = safe_calloc(sector_count, sizeof(Array));

    for (int i = 0; i < sector_count; i++) {
        Sector *s = sectors[i];

        Vec **s_vecs = s->vecs;
        int s_vec_count = s->vec_count;

        Array *s_temp_inside_list = &sector_inside_lists[i];
        array_init(s_temp_inside_list, 0);

        for (int k = 0; k < sector_count; k++) {
            if (k == i) {
                continue;
            }

            Sector *o = sectors[k];

            Vec **o_vecs = o->vecs;
            int o_vec_count = o->vec_count;

            bool contained = true;

            for (int w = 0; w < o_vec_count; w++) {

                for (int c = 0; c < s_vec_count; c++) {
                    if (s_vecs[c] == o_vecs[w]) {
                        contained = false;
                        goto label_contained;
                    }
                }

                if (!sector_contains(s, o_vecs[w]->x, o_vecs[w]->y)) {
                    contained = false;
                    goto label_contained;
                }
            }

        label_contained:
            if (contained) {
                array_push(s_temp_inside_list, o);
            }
        }
    }
}
