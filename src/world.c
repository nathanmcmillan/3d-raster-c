/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "world.h"

World *new_world() {
    return safe_calloc(1, sizeof(World));
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
