/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "sector.h"

Cell *CELLS = NULL;
int CELL_COLUMNS = 0;
int CELL_ROWS = 0;

void CellAddLine(Cell *this, Line *line) {
    if (this->line_count == 0) {
        this->lines = Malloc(sizeof(Line *));
        this->lines[0] = line;
        this->line_count = 1;
        return;
    }
    int count = this->line_count;
    Line **lines = this->lines;
    for (int i = 0; i < count; i++) {
        if (lines[i] == line)
            return;
    }
    this->lines = Realloc(this->lines, (count + 1) * sizeof(Line *));
    this->lines[count] = line;
    this->line_count++;
}

void CellAddThing(Cell *this, Thing *thing) {
    if (this->thing_capacity == 0) {
        this->things = Malloc(sizeof(Thing *));
        this->things[0] = thing;
        this->thing_capacity = 1;
        this->thing_count = 1;
        return;
    }
    if (this->thing_count == this->thing_capacity) {
        this->thing_capacity += 8;
        this->things = Realloc(this->things, this->thing_capacity * sizeof(Thing *));
    }
    this->things[this->thing_count] = thing;
    this->thing_count++;
}

void CellRemoveThing(Cell *this, Thing *thing) {
    int count = this->thing_count;
    Thing **things = this->things;
    for (int i = 0; i < count; i++) {
        if (things[i] == thing) {
            things[i] = things[count - 1];
            this->thing_count--;
            return;
        }
    }
}

void CellAddParticle(Cell *this, Particle *particle) {
    if (this->particle_capacity == 0) {
        this->particles = Malloc(sizeof(Particle *));
        this->particles[0] = particle;
        this->particle_capacity = 1;
        this->particle_count = 1;
        return;
    }
    if (this->particle_count == this->particle_capacity) {
        this->particle_capacity += 8;
        this->particles = Realloc(this->particles, this->particle_capacity * sizeof(Particle *));
    }
    this->particles[this->particle_count] = particle;
    this->particle_count++;
}

void CellRemoveParticle(Cell *this, Particle *particle) {
    int count = this->particle_count;
    Particle **particles = this->particles;
    for (int i = 0; i < count; i++) {
        if (particles[i] == particle) {
            particles[i] = particles[count - 1];
            this->particle_count--;
            return;
        }
    }
}
