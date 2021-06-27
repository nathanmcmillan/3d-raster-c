/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "world.h"

World *new_world() {
    return safe_calloc(1, sizeof(World));
}

void world_clear(World *this) {
    (void *)this;
}

void world_add_thing(World *this, Thing *t) {

    if (this->thing_cap == 0) {
        this->things = safe_malloc(sizeof(Thing *));
        this->things[0] = t;
        this->thing_cap = 1;
        this->thing_count = 1;
    } else {

        if (this->thing_count == this->thing_cap) {
            this->thing_cap += 8;
            this->things = safe_realloc(this->things, this->thing_cap * sizeof(Thing *));
        }
        this->things[this->thing_count] = t;
        this->thing_count++;
    }

    if (this->thing_sprites_cap == 0) {
        this->thing_sprites = safe_malloc(sizeof(Thing *));
        this->thing_sprites[0] = t;
        this->thing_sprites_cap = 1;
        this->thing_sprites_count = 1;
    } else {

        if (this->thing_sprites_count == this->thing_sprites_cap) {
            this->thing_sprites_cap += 8;
            this->thing_sprites = safe_realloc(this->things, this->thing_sprites_cap * sizeof(Thing *));
        }
        this->thing_sprites[this->thing_sprites_count] = t;
        this->thing_sprites_count++;
    }
}

void world_remove_thing(World *this, Thing *t) {

    int len = this->thing_count;
    Thing **things = this->things;
    for (int i = 0; i < len; i++) {
        if (things[i] == t) {
            things[i] = things[len - 1];
            this->thing_count--;
            break;
        }
    }

    len = this->thing_sprites_count;
    things = this->thing_sprites;
    for (int i = 0; i < len; i++) {
        if (things[i] == t) {
            things[i] = things[len - 1];
            this->thing_sprites_count--;
            break;
        }
    }
}

void world_add_particle(World *this, Particle *t) {

    if (this->particle_cap == 0) {
        this->particles = safe_malloc(sizeof(Particle *));
        this->particles[0] = t;
        this->particle_cap = 1;
        this->particle_count = 1;
        return;
    }

    if (this->particle_count == this->particle_cap) {
        this->particle_cap += 8;
        this->particles = safe_realloc(this->particles, this->particle_cap * sizeof(Particle *));
    }

    this->particles[this->particle_count] = t;
    this->particle_count++;
}

void world_remove_particle(World *this, Particle *t) {

    int len = this->particle_count;
    Particle **particles = this->particles;
    for (int i = 0; i < len; i++) {
        if (particles[i] == t) {
            particles[i] = particles[len - 1];
            this->particle_count--;
            return;
        }
    }
}

void world_add_decal(World *this, Decal *t) {

    if (this->decal_cap == 0) {
        this->decals = safe_malloc(sizeof(Decal *));
        this->decals[0] = t;
        this->decal_cap = 1;
        this->decal_count = 1;
        return;
    }

    if (this->decal_count == this->decal_cap) {
        this->decal_cap += 8;
        this->decals = safe_realloc(this->decals, this->decal_cap * sizeof(Decal *));
    }

    this->decals[this->decal_count] = t;
    this->decal_count++;
}

void world_remove_decal(World *this, Decal *t) {

    int len = this->decal_count;
    Decal **decals = this->decals;
    for (int i = 0; i < len; i++) {
        if (decals[i] == t) {
            decals[i] = decals[len - 1];
            this->decal_count--;
            return;
        }
    }
}

void world_add_sector(World *this, Sector *s) {

    if (this->sector_cap == 0) {
        this->sectors = safe_malloc(sizeof(Sector *));
        this->sectors[0] = s;
        this->sector_cap = 1;
        this->sector_count = 1;
        return;
    }

    if (this->sector_count == this->sector_cap) {
        this->sector_cap += 8;
        this->sectors = safe_realloc(this->sectors, this->sector_cap * sizeof(Sector *));
    }

    this->sectors[this->sector_count] = s;
    this->sector_count++;
}

Sector *world_find_sector(World *this, float x, float y) {
    for (int i = 0; i < this->sector_count; i++) {
        Sector *s = this->sectors[i];
        if (s->outside != NULL)
            continue;
        if (sector_contains(s, x, y))
            return sector_find(s, x, y);
    }
    return NULL;
}

static void build_cell_lines(World *this, Line *line) {

    double dx = fabs(line->b->x - line->a->x);
    double dy = fabs(line->b->y - line->a->y);

    int x = (int)floor(line->a->x);
    int y = (int)floor(line->a->y);

    int n = 1;
    double error;
    int x_inc;
    int y_inc;

    if (dx == 0) {
        x_inc = 0;
        error = DBL_MAX;
    } else if (line->b->x > line->a->x) {
        x_inc = 1;
        n += (int)floor(line->b->x) - x;
        error = (floor(line->a->x) + 1 - line->a->x) * dy;
    } else {
        x_inc = -1;
        n += x - (int)(floor(line->b->x));
        error = (line->a->x - floor(line->a->x)) * dy;
    }

    if (dy == 0) {
        y_inc = 0;
        error = DBL_MIN;
    } else if (line->b->y > line->a->y) {
        y_inc = 1;
        n += (int)floor(line->b->y) - y;
        error -= (floor(line->a->y) + 1 - line->a->y) * dx;
    } else {
        y_inc = -1;
        n += y - (int)floor(line->b->y);
        error -= (line->a->y - floor(line->a->y)) * dx;
    }

    while (n > 0) {
        Cell *c = &this->cells[(x >> WORLD_CELL_SHIFT) + (y >> WORLD_CELL_SHIFT) * this->columns];
        cell_add_line(c, line);

        if (error > 0) {
            y += y_inc;
            error -= dx;
        } else {
            x += x_inc;
            error += dy;
        }

        n--;
    }
}

static void build_lines(World *this, Sector *sec) {
    int line_count = sec->line_count;

    if (line_count == 0) {
        return;
    }

    float bottom = sec->bottom;
    float floor = sec->floor;
    float ceil = sec->ceiling;
    float top = sec->top;

    Sector *plus;
    Sector *minus;

    if (sec->outside == NULL) {
        plus = NULL;
        minus = sec;
    } else {
        plus = sec;
        minus = sec->outside;
    }

    Line **lines = sec->lines;

    float u = 0.0f;

    for (int i = 0; i < line_count; i++) {

        Line *line = lines[i];

        build_cell_lines(this, line);

        line_set_sectors(line, plus, minus);

        float x = line->a->x - line->b->x;
        float y = line->a->y - line->b->y;
        float s = u + sqrtf(x * x + y * y) * WORLD_SCALE;

        if (line->bottom != NULL) {
            wall_set(line->bottom, bottom, floor, u, bottom * WORLD_SCALE, s, floor * WORLD_SCALE);
        }

        if (line->middle != NULL) {
            wall_set(line->middle, floor, ceil, u, floor * WORLD_SCALE, s, ceil * WORLD_SCALE);
        }

        if (line->top != NULL) {
            wall_set(line->top, ceil, top, u, ceil * WORLD_SCALE, s, top * WORLD_SCALE);
        }

        u = s;
    }
}

void world_build(World *this, Array *lines) {

    this->lines = (Line **)array_copy_items(lines);
    this->line_count = (int)array_size(lines);

    float top = 0.0f;
    float right = 0.0f;

    Sector **sectors = this->sectors;
    int sector_count = this->sector_count;

    for (int i = 0; i < sector_count; i++) {
        Sector *s = sectors[i];
        int vector_count = s->vec_count;
        Vec **vecs = s->vecs;
        for (int k = 0; k < vector_count; k++) {
            Vec *vec = vecs[k];
            if (vec->y > top) {
                top = vec->y;
            }
            if (vec->x > right) {
                right = vec->x;
            }
        }
    }

    const int size = 1 << WORLD_CELL_SHIFT;

    this->rows = (int)ceil(top / size);
    this->columns = (int)ceil(right / size);
    this->cell_count = this->rows * this->columns;
    this->cells = safe_calloc(this->cell_count, sizeof(Cell));

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

    for (int i = 0; i < sector_count; i++) {
        Sector *s = sectors[i];
        Array *s_temp_inside_list = &sector_inside_lists[i];
        int s_inside_count = (int)s_temp_inside_list->length;

        Array *dead = new_array(0);

        for (int k = 0; k < s_inside_count; k++) {
            Sector *o = s_temp_inside_list->items[k];
            int index_of_o;
            for (index_of_o = 0; index_of_o < sector_count; index_of_o++) {
                if (sectors[index_of_o] == o) {
                    break;
                }
            }
            Array *o_inside_list = &sector_inside_lists[index_of_o];
            int o_inside_count = (int)o_inside_list->length;
            for (int w = 0; w < o_inside_count; w++) {
                array_push(dead, o_inside_list->items[w]);
            }
        }

        for (int k = 0; k < (int)array_size(dead); k++) {
            array_remove(s_temp_inside_list, dead->items[k]);
        }

        for (int k = 0; k < s_inside_count; k++) {
            ((Sector *)s_temp_inside_list->items[k])->outside = s;
        }

        array_delete(dead);

        s->inside = (Sector **)array_copy_items(s_temp_inside_list);
        s->inside_count = (int)s_temp_inside_list->length;
    }

    for (int i = 0; i < sector_count; i++) {
        array_release(&sector_inside_lists[i]);
    }

    free(sector_inside_lists);

    for (int i = 0; i < sector_count; i++) {
        triangulate_sector(sectors[i], WORLD_SCALE);
    }

    for (int i = 0; i < sector_count; i++) {
        build_lines(this, sectors[i]);
    }
}

void world_update(World *this) {

    int thing_count = this->thing_count;
    Thing **things = this->things;
    for (int i = 0; i < thing_count; i++) {
        Thing *t = things[i];
        t->update(t);
    }

    int particle_count = this->particle_count;
    Particle **particles = this->particles;
    for (int i = 0; i < particle_count; i++) {
        Particle *p = particles[i];
        if (p->update(p)) {
            particles[i] = particles[particle_count - 1];
            particles[particle_count - 1] = NULL;
            this->particle_count--;
            particle_count--;
            i--;
        }
    }
}
