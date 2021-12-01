/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "sector.h"

Sector *new_sector(int id, Line **lines, int line_count, float floor, float ceiling, int floor_image, int ceiling_image) {
    Sector *s = safe_calloc(1, sizeof(Sector));
    s->id = id;
    int vec_count = line_count - 1;
    s->vecs = safe_calloc(vec_count, sizeof(Vec *));
    s->vec_count = vec_count;
    for (int v = 0; v < vec_count; v++) {
        s->vecs[v] = lines[v]->a;
    }
    s->lines = lines;
    s->line_count = line_count;
    s->floor = floor;
    s->ceiling = ceiling;
    s->floor_image = floor_image;
    s->ceiling_image = ceiling_image;
    return s;
}

bool sector_contains(Sector *this, float x, float y) {
    bool odd = false;
    Vec **vecs = this->vecs;
    int count = this->vec_count;
    int n = count - 1;
    for (int v = 0; v < count; v++) {
        Vec *a = vecs[v];
        Vec *b = vecs[n];
        if (a->y > y != b->y > y) {
            float value = (b->x - a->x) * (y - a->y) / (b->y - a->y) + a->x;
            if (x < value) {
                odd = !odd;
            }
        }
        n = v;
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

Sector *search_for(Sector *this, float x, float y) {
    if (sector_contains(this, x, y)) {
        return sector_find(this, x, y);
    }
    // ...
    return NULL;
}

bool sector_has_floor(Sector *this) {
    return this->floor_image != SECTOR_NO_SURFACE;
}

bool sector_has_ceiling(Sector *this) {
    return this->ceiling_image != SECTOR_NO_SURFACE;
}

void sector_add_inside(Sector *this, Sector *inside) {
    if (this->inside_count == 0) {
        this->inside = safe_malloc(sizeof(Sector *));
    } else {
        this->inside = safe_realloc(this->inside, (this->inside_count + 1) * sizeof(Sector *));
    }
    this->inside[this->inside_count] = inside;
    this->inside_count++;
}

void sector_delete_inside(Set *set, Sector *in) {
    for (int i = 0; i < in->inside_count; i++) {
        Sector *inside = in->inside[i];
        set_add(set, inside);
        sector_delete_inside(set, inside);
    }
}

void sector_inside_outside(Sector **sectors, int sector_count) {
    for (int sa = 0; sa < sector_count; sa++) {
        Sector *sector = sectors[sa];
        Vec **sector_vecs = sector->vecs;
        int sector_vec_count = sector->vec_count;
        for (int so = 0; so < sector_count; so++) {
            if (sa == so) {
                continue;
            }
            Sector *other = sectors[so];
            Vec **other_vecs = other->vecs;
            int other_vec_count = other->vec_count;
            int inside = 0;
            int outside = 0;
            for (int v = 0; v < other_vec_count; v++) {
                Vec *vec = other_vecs[v];
                bool shared = false;
                for (int c = 0; c < sector_vec_count; c++) {
                    if (sector_vecs[c] == vec) {
                        shared = true;
                        break;
                    }
                }
                if (shared) {
                    continue;
                }
                if (sector_contains(sector, vec->x, vec->y)) {
                    inside++;
                } else {
                    outside++;
                }
            }
            if (outside == 0 && inside > 0) {
                sector_add_inside(sector, other);
            }
        }
    }
    Set *dead = new_address_set();
    for (int s = 0; s < sector_count; s++) {
        Sector *sector = sectors[s];
        Sector **inside = sector->inside;
        int inside_count = sector->inside_count;
        set_clear(dead);
        for (int i = 0; i < inside_count; i++) {
            sector_delete_inside(dead, inside[i]);
        }
        SetIterator iterator = new_set_iterator(dead);
        while (set_iterator_has_next(&iterator)) {
            Sector *delete = set_iterator_next(&iterator);
            for (int i = 0; i < inside_count; i++) {
                if (inside[i] == delete) {
                    inside_count--;
                    inside[i] = inside[inside_count];
                    break;
                }
            }
        }
        if (inside_count != sector->inside_count) {
            sector->inside_count = inside_count;
            sector->inside = safe_realloc(sector->inside, inside_count * sizeof(Sector *));
        }
        for (int i = 0; i < inside_count; i++) {
            sector->inside[i]->outside = sector;
        }
    }
    set_delete(dead);
}

static void sector_line_direction(Sector *sector, Line *line) {
    Vec *a = line->a;
    Vec **vecs = sector->vecs;
    int vector_count = sector->vec_count;
    for (int v = 0; v < vector_count; v++) {
        Vec *vec = vecs[v];
        if (vec != a) {
            continue;
        }
        v++;
        if (v == vector_count) {
            v = 0;
        }
        if (vecs[v] == line->b) {
            line->front = sector;
        } else {
            line->back = sector;
        }
        return;
    }
}

bool sector_has_neighbor(Sector *sector, Sector *other) {
    Sector **neighbors = sector->neighbors;
    int count = sector->neighbor_count;
    for (int n = 0; n < count; n++) {
        if (neighbors[n] == other) {
            return true;
        }
    }
    return false;
}

void sector_add_neighbor(Sector *this, Sector *neighbor) {
    if (this->neighbor_count == 0) {
        this->neighbors = safe_malloc(sizeof(Sector *));
    } else {
        this->neighbors = safe_realloc(this->neighbors, (this->neighbor_count + 1) * sizeof(Sector *));
    }
    this->neighbors[this->neighbor_count] = neighbor;
    this->neighbor_count++;
}

void sector_neighbors(Sector **sectors, int sector_count, Line **lines, int line_count) {
    for (int s = 0; s < sector_count; s++) {
        Sector *sector = sectors[s];
        for (int w = 0; w < sector->line_count; w++) {
            sector_line_direction(sector, sector->lines[w]);
        }
    }
    for (int w = 0; w < line_count; w++) {
        Line *line = lines[w];
        Sector *front = line->front;
        Sector *back = line->back;
        if (front != NULL && back != NULL) {
            if (!sector_has_neighbor(front, back)) {
                sector_add_neighbor(front, back);
            }
            if (!sector_has_neighbor(back, front)) {
                sector_add_neighbor(back, front);
            }
        }
    }
    for (int s = 0; s < sector_count; s++) {
        Sector *sector = sectors[s];
        Sector *outside = sector->outside;
        if (outside == NULL) {
            continue;
        }
        if (!sector_has_neighbor(sector, outside)) {
            sector_add_neighbor(sector, outside);
        }
        if (!sector_has_neighbor(outside, sector)) {
            sector_add_neighbor(outside, sector);
        }
        for (int w = 0; w < sector->line_count; w++) {
            Line *line = sector->lines[w];
            if (line->front == NULL) {
                line->front = outside;
            } else if (line->back == NULL) {
                line->back = outside;
            }
        }
    }
}
