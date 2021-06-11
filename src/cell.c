/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "world.h"

void cell_add_line(cell *this, line *ld) {
    if (this->line_count == 0) {
        this->lines = safe_malloc(sizeof(line *));
        this->lines[0] = ld;
        this->line_count = 1;
        return;
    }

    int len = this->line_count;
    line **lines = this->lines;
    for (int i = 0; i < len; i++) {
        if (lines[i] == ld) {
            return;
        }
    }

    this->lines = safe_realloc(this->lines, (len + 1) * sizeof(line *));
    this->lines[len] = ld;
    this->line_count++;
}

void cell_add_thing(cell *this, thing *t) {
    if (this->thing_cap == 0) {
        this->things = safe_malloc(sizeof(thing *));
        this->things[0] = t;
        this->thing_cap = 1;
        this->thing_count = 1;
        return;
    }

    if (this->thing_count == this->thing_cap) {
        this->thing_cap += 8;
        this->things = safe_realloc(this->things, this->thing_cap * sizeof(thing *));
    }

    this->things[this->thing_count] = t;
    this->thing_count++;
}

void cell_remove_thing(cell *this, thing *t) {
    int len = this->thing_count;
    thing **things = this->things;
    for (int i = 0; i < len; i++) {
        if (things[i] == t) {
            things[i] = things[len - 1];
            this->thing_count--;
            return;
        }
    }
}

void cell_add_particle(cell *this, particle *t) {
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

void cell_remove_particle(cell *this, particle *t) {
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

void cell_add_decal(cell *this, decal *t) {
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

void cell_remove_decal(cell *this, decal *t) {
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
