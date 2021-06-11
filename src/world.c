/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "world.h"

world *new_world() {
    return safe_calloc(1, sizeof(world));
}

void world_add_thing(world *this, thing *t) {

    if (this->thing_cap == 0) {
        this->things = safe_malloc(sizeof(thing *));
        this->things[0] = t;
        this->thing_cap = 1;
        this->thing_count = 1;
    } else {

        if (this->thing_count == this->thing_cap) {
            this->thing_cap += 8;
            this->things = safe_realloc(this->things, this->thing_cap * sizeof(thing *));
        }
        this->things[this->thing_count] = t;
        this->thing_count++;
    }

    if (this->thing_sprites_cap == 0) {
        this->thing_sprites = safe_malloc(sizeof(thing *));
        this->thing_sprites[0] = t;
        this->thing_sprites_cap = 1;
        this->thing_sprites_count = 1;
    } else {

        if (this->thing_sprites_count == this->thing_sprites_cap) {
            this->thing_sprites_cap += 8;
            this->thing_sprites = safe_realloc(this->things, this->thing_sprites_cap * sizeof(thing *));
        }
        this->thing_sprites[this->thing_sprites_count] = t;
        this->thing_sprites_count++;
    }
}

void world_remove_thing(world *this, thing *t) {

    int len = this->thing_count;
    thing **things = this->things;
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

void world_add_particle(world *this, particle *t) {

    if (this->particle_cap == 0) {
        this->particles = safe_malloc(sizeof(particle *));
        this->particles[0] = t;
        this->particle_cap = 1;
        this->particle_count = 1;
        return;
    }

    if (this->particle_count == this->particle_cap) {
        this->particle_cap += 8;
        this->particles = safe_realloc(this->particles, this->particle_cap * sizeof(particle *));
    }

    this->particles[this->particle_count] = t;
    this->particle_count++;
}

void world_remove_particle(world *this, particle *t) {

    int len = this->particle_count;
    particle **particles = this->particles;
    for (int i = 0; i < len; i++) {
        if (particles[i] == t) {
            particles[i] = particles[len - 1];
            this->particle_count--;
            return;
        }
    }
}

void world_add_decal(world *this, decal *t) {

    if (this->decal_cap == 0) {
        this->decals = safe_malloc(sizeof(decal *));
        this->decals[0] = t;
        this->decal_cap = 1;
        this->decal_count = 1;
        return;
    }

    if (this->decal_count == this->decal_cap) {
        this->decal_cap += 8;
        this->decals = safe_realloc(this->decals, this->decal_cap * sizeof(decal *));
    }

    this->decals[this->decal_count] = t;
    this->decal_count++;
}

void world_remove_decal(world *this, decal *t) {

    int len = this->decal_count;
    decal **decals = this->decals;
    for (int i = 0; i < len; i++) {
        if (decals[i] == t) {
            decals[i] = decals[len - 1];
            this->decal_count--;
            return;
        }
    }
}

void world_add_sector(world *this, sector *s) {

    if (this->sector_cap == 0) {
        this->sectors = safe_malloc(sizeof(sector *));
        this->sectors[0] = s;
        this->sector_cap = 1;
        this->sector_count = 1;
        return;
    }

    if (this->sector_count == this->sector_cap) {
        this->sector_cap += 8;
        this->sectors = safe_realloc(this->sectors, this->sector_cap * sizeof(sector *));
    }

    this->sectors[this->sector_count] = s;
    this->sector_count++;
}

sector *world_find_sector(world *this, float x, float y) {
    for (int i = 0; i < this->sector_count; i++) {
        sector *s = this->sectors[i];
        if (s->outside != NULL)
            continue;
        if (sector_contains(s, x, y))
            return sector_find(s, x, y);
    }
    return NULL;
}

void world_update(world *this) {

    int thing_count = this->thing_count;
    thing **things = this->things;
    for (int i = 0; i < thing_count; i++) {
        thing *t = things[i];
        t->update(t);
    }

    int particle_count = this->particle_count;
    particle **particles = this->particles;
    for (int i = 0; i < particle_count; i++) {
        particle *p = particles[i];
        if (p->update(p)) {
            particles[i] = particles[particle_count - 1];
            particles[particle_count - 1] = NULL;
            this->particle_count--;
            particle_count--;
            i--;
        }
    }
}
