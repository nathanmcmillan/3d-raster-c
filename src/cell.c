/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "world.h"

void cell_add_line(Cell *this, Line *ld) {
    if (this->line_count == 0) {
        this->lines = safe_malloc(sizeof(Line *));
        this->lines[0] = ld;
        this->line_count = 1;
        return;
    }

    int len = this->line_count;
    Line **lines = this->lines;
    for (int i = 0; i < len; i++) {
        if (lines[i] == ld) {
            return;
        }
    }

    this->lines = safe_realloc(this->lines, (len + 1) * sizeof(Line *));
    this->lines[len] = ld;
    this->line_count++;
}

void cell_add_thing(Cell *this, Thing *t) {
    if (this->thing_cap == 0) {
        this->things = safe_malloc(sizeof(Thing *));
        this->things[0] = t;
        this->thing_cap = 1;
        this->thing_count = 1;
        return;
    }

    if (this->thing_count == this->thing_cap) {
        this->thing_cap += 8;
        this->things = safe_realloc(this->things, this->thing_cap * sizeof(Thing *));
    }

    this->things[this->thing_count] = t;
    this->thing_count++;
}

void cell_remove_thing(Cell *this, Thing *t) {
    int len = this->thing_count;
    Thing **things = this->things;
    for (int i = 0; i < len; i++) {
        if (things[i] == t) {
            things[i] = things[len - 1];
            this->thing_count--;
            return;
        }
    }
}

void cell_add_particle(Cell *this, Particle *t) {
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

void cell_remove_particle(Cell *this, Particle *t) {
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

void cell_add_decal(Cell *this, Decal *t) {
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

void cell_remove_decal(Cell *this, Decal *t) {
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
