/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "world.h"

World *new_world() {
    return Calloc(1, sizeof(World));
}

int world_cell(float f) {
    return ((int)floorf(f)) >> WORLD_CELL_SHIFT;
}

float world_float_cell(float f) {
    return f / (1 << WORLD_CELL_SHIFT);
}

void WorldClear(World *world) {
    Free(world->sectors_visited);
    Free(world->lines_visited);
}

void world_add_thing(World *this, Thing *t) {
    if (this->thing_cap == 0) {
        this->things = Malloc(sizeof(Thing *));
        this->things[0] = t;
        this->thing_cap = 1;
        this->thing_count = 1;
    } else {
        if (this->thing_count == this->thing_cap) {
            this->thing_cap += 8;
            this->things = Realloc(this->things, this->thing_cap * sizeof(Thing *));
        }
        this->things[this->thing_count] = t;
        this->thing_count++;
    }
}

void world_remove_thing(World *this, Thing *t) {
    int count = this->thing_count;
    Thing **things = this->things;
    for (int i = 0; i < count; i++) {
        if (things[i] == t) {
            things[i] = things[count - 1];
            this->thing_count--;
            return;
        }
    }
}

void world_add_particle(World *this, Particle *t) {
    if (this->particle_cap == 0) {
        this->particles = Malloc(sizeof(Particle *));
        this->particles[0] = t;
        this->particle_cap = 1;
        this->particle_count = 1;
    } else {
        if (this->particle_count == this->particle_cap) {
            this->particle_cap += 8;
            this->particles = Realloc(this->particles, this->particle_cap * sizeof(Particle *));
        }
        this->particles[this->particle_count] = t;
        this->particle_count++;
    }
}

void world_remove_particle(World *this, Particle *t) {
    int count = this->particle_count;
    Particle **particles = this->particles;
    for (int i = 0; i < count; i++) {
        if (particles[i] == t) {
            particles[i] = particles[count - 1];
            this->particle_count--;
            return;
        }
    }
}

void world_add_decal(World *this, Decal *t) {
    if (this->decal_cap == 0) {
        this->decals = Malloc(sizeof(Decal *));
        this->decals[0] = t;
        this->decal_cap = 1;
        this->decal_count = 1;
    } else {
        if (this->decal_count == this->decal_cap) {
            this->decal_cap += 8;
            this->decals = Realloc(this->decals, this->decal_cap * sizeof(Decal *));
        }
        this->decals[this->decal_count] = t;
        this->decal_count++;
    }
}

void world_remove_decal(World *this, Decal *t) {
    int count = this->decal_count;
    Decal **decals = this->decals;
    for (int i = 0; i < count; i++) {
        if (decals[i] == t) {
            decals[i] = decals[count - 1];
            this->decal_count--;
            return;
        }
    }
}

void world_add_sector(World *this, Sector *s) {
    if (this->sector_cap == 0) {
        this->sectors = Malloc(sizeof(Sector *));
        this->sectors[0] = s;
        this->sector_cap = 1;
        this->sector_count = 1;
    } else {
        if (this->sector_count == this->sector_cap) {
            this->sector_cap += 8;
            this->sectors = Realloc(this->sectors, this->sector_cap * sizeof(Sector *));
        }
        this->sectors[this->sector_count] = s;
        this->sector_count++;
    }
}

Sector *world_find_sector(World *this, float x, float y) {
    int i = this->sector_count;
    while (i-- != 0) {
        Sector *sector = this->sectors[i];
        if (sector->outside != NULL) {
            continue;
        } else if (SectorContains(sector, x, y)) {
            return SectorFind(sector, x, y);
        }
    }
    return NULL;
}

static void world_build_cell_lines(World *this, Line *line) {
    float xf = world_float_cell(line->a->x);
    float yf = world_float_cell(line->a->y);
    float dx = fabsf(world_float_cell(line->b->x) - xf);
    float dy = fabsf(world_float_cell(line->b->y) - yf);
    int x = world_cell(line->a->x);
    int y = world_cell(line->a->y);
    int xb = world_cell(line->b->x);
    int yb = world_cell(line->b->y);
    int n = 1;
    float error;
    int increment_x;
    int increment_y;
    if (FLOAT_ZERO(dx)) {
        increment_x = 0;
        error = FLT_MAX;
    } else if (line->b->x > line->a->x) {
        increment_x = 1;
        n += xb - x;
        error = (x + 1.0f - xf) * dy;
    } else {
        increment_x = -1;
        n += x - xb;
        error = (xf - x) * dy;
    }
    if (FLOAT_ZERO(dy)) {
        increment_y = 0;
        error = FLT_MIN;
    } else if (line->b->y > line->a->y) {
        increment_y = 1;
        n += yb - y;
        error -= (y + 1.0f - yf) * dx;
    } else {
        increment_y = -1;
        n += y - yb;
        error -= (yf - y) * dx;
    }
    while (n > 0) {
        Cell *c = &this->cells[(x >> WORLD_CELL_SHIFT) + (y >> WORLD_CELL_SHIFT) * this->columns];
        cell_add_line(c, line);
        if (error > 0.0) {
            y += increment_y;
            error -= dx;
        } else {
            x += increment_x;
            error += dy;
        }
        n--;
    }
}

void WorldBuild(World *this, Line **lines, int line_count) {
    printf("building world...\n");
    this->lines = lines;
    this->line_count = line_count;
    float top = 0.0f;
    float right = 0.0f;
    Sector **sectors = this->sectors;
    int sector_count = this->sector_count;
    for (int s = 0; s < sector_count; s++) {
        Sector *sector = sectors[s];
        int vector_count = sector->vec_count;
        Vec **vecs = sector->vecs;
        for (int v = 0; v < vector_count; v++) {
            Vec *vec = vecs[v];
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
    this->cells = Calloc(this->cell_count, sizeof(Cell));
    SectorInsideOutside(this->sectors, this->sector_count);
    SectorNeighbors(this->sectors, this->sector_count, this->lines, this->line_count);
    for (int i = 0; i < this->line_count; i++) {
        world_build_cell_lines(this, this->lines[i]);
    }
    this->sectors_visited = calloc(sector_count, sizeof(u32));
    this->lines_visited = calloc(this->line_count, sizeof(u32));
    printf("done building world...\n");
}

void WorldUpdate(World *world) {
    Thing **things = world->things;
    int t = world->thing_count;
    while (t-- != 0) {
        Thing *thing = things[t];
        thing->update(thing);
    }

    Particle **particles = world->particles;
    int p = world->particle_count;
    while (p-- != 0) {
        Particle *particle = particles[p];
        if (particle->update(particle)) {
            world->particle_count--;
            particles[p] = particles[world->particle_count];
        }
    }

    world->tick++;
}

void world_delete(World *world) {
    WorldClear(world);
    Free(world);
}
