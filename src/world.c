/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "sector.h"

World *WORLD = NULL;

void WorldInit() {
    WORLD = Calloc(1, sizeof(World));
}

int WorldCell(float f) {
    return ((int)floorf(f)) >> WORLD_CELL_SHIFT;
}

float WorldFloatCell(float f) {
    return f / (1 << WORLD_CELL_SHIFT);
}

void WorldClear() {
    Free(WORLD->sectors);
    Free(CELLS);
}

void WorldAddThing(Thing *thing) {
    if (WORLD->thing_capacity == 0) {
        WORLD->things = Malloc(sizeof(Thing *));
        WORLD->things[0] = thing;
        WORLD->thing_capacity = 1;
        WORLD->thing_count = 1;
    } else {
        if (WORLD->thing_count == WORLD->thing_capacity) {
            WORLD->thing_capacity += 8;
            WORLD->things = Realloc(WORLD->things, WORLD->thing_capacity * sizeof(Thing *));
        }
        WORLD->things[WORLD->thing_count] = thing;
        WORLD->thing_count++;
    }
}

void WorldRemoveThing(Thing *thing) {
    int count = WORLD->thing_count;
    Thing **things = WORLD->things;
    for (int i = 0; i < count; i++) {
        if (things[i] == thing) {
            things[i] = things[count - 1];
            WORLD->thing_count--;
            return;
        }
    }
}

void WorldAddParticle(Particle *particle) {
    if (WORLD->particle_capacity == 0) {
        WORLD->particles = Malloc(sizeof(Particle *));
        WORLD->particles[0] = particle;
        WORLD->particle_capacity = 1;
        WORLD->particle_count = 1;
    } else {
        if (WORLD->particle_count == WORLD->particle_capacity) {
            WORLD->particle_capacity += 8;
            WORLD->particles = Realloc(WORLD->particles, WORLD->particle_capacity * sizeof(Particle *));
        }
        WORLD->particles[WORLD->particle_count] = particle;
        WORLD->particle_count++;
    }
}

void WorldRemoveParticle(Particle *particle) {
    int count = WORLD->particle_count;
    Particle **particles = WORLD->particles;
    for (int i = 0; i < count; i++) {
        if (particles[i] == particle) {
            particles[i] = particles[count - 1];
            WORLD->particle_count--;
            return;
        }
    }
}

void WorldAddDecal(Decal *decal) {
    if (WORLD->decal_capacity == 0) {
        WORLD->decals = Malloc(sizeof(Decal *));
        WORLD->decals[0] = decal;
        WORLD->decal_capacity = 1;
        WORLD->decal_count = 1;
    } else {
        if (WORLD->decal_count == WORLD->decal_capacity) {
            WORLD->decal_capacity += 8;
            WORLD->decals = Realloc(WORLD->decals, WORLD->decal_capacity * sizeof(Decal *));
        }
        WORLD->decals[WORLD->decal_count] = decal;
        WORLD->decal_count++;
    }
}

void WorldRemoveDecal(Decal *decal) {
    int count = WORLD->decal_count;
    Decal **decals = WORLD->decals;
    for (int i = 0; i < count; i++) {
        if (decals[i] == decal) {
            decals[i] = decals[count - 1];
            WORLD->decal_count--;
            return;
        }
    }
}

void WorldAddSector(Sector *sector) {
    if (WORLD->sector_capacity == 0) {
        WORLD->sectors = Malloc(sizeof(Sector *));
        WORLD->sectors[0] = sector;
        WORLD->sector_capacity = 1;
        WORLD->sector_count = 1;
    } else {
        if (WORLD->sector_count == WORLD->sector_capacity) {
            WORLD->sector_capacity += 8;
            WORLD->sectors = Realloc(WORLD->sectors, WORLD->sector_capacity * sizeof(Sector *));
        }
        WORLD->sectors[WORLD->sector_count] = sector;
        WORLD->sector_count++;
    }
}

Sector *WorldFindSector(float x, float z) {
    Sector **sectors = WORLD->sectors;
    int i = WORLD->sector_count;
    while (i-- != 0) {
        Sector *sector = sectors[i];
        if (sector->outside != NULL)
            continue;
        else if (SectorContains(sector, x, z))
            return SectorFind(sector, x, z);
    }
    return NULL;
}

static void WorldBuildCellLines(Line *line) {
    float xf = WorldFloatCell(line->a->x);
    float yf = WorldFloatCell(line->a->y);
    float dx = fabsf(WorldFloatCell(line->b->x) - xf);
    float dy = fabsf(WorldFloatCell(line->b->y) - yf);
    int x = WorldCell(line->a->x);
    int y = WorldCell(line->a->y);
    int xb = WorldCell(line->b->x);
    int yb = WorldCell(line->b->y);
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
        Cell *c = &CELLS[(x >> WORLD_CELL_SHIFT) + (y >> WORLD_CELL_SHIFT) * CELL_COLUMNS];
        CellAddLine(c, line);
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

void WorldBuild(Line **lines, int line_count) {
    printf("building world...\n");
    WORLD->lines = lines;
    WORLD->line_count = line_count;
    float top = 0.0f;
    float right = 0.0f;
    Sector **sectors = WORLD->sectors;
    int sector_count = WORLD->sector_count;
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
    CELL_ROWS = (int)ceil(top / size);
    CELL_COLUMNS = (int)ceil(right / size);
    CELLS = Calloc(CELL_ROWS * CELL_COLUMNS, sizeof(Cell));
    SectorInsideOutside(WORLD->sectors, WORLD->sector_count);
    SectorNeighbors(WORLD->sectors, WORLD->sector_count, WORLD->lines, WORLD->line_count);
    for (int i = 0; i < WORLD->line_count; i++) {
        WorldBuildCellLines(WORLD->lines[i]);
    }
    printf("done building world...\n");
}

Thing *WorldSpawnEntity(String *id, float x, float y, float z) {
    (void)id;
    Thing *thing = NewThing(x, y, z);
    return thing;
}

void WorldUpdate() {
    Thing **things = WORLD->things;
    int t = WORLD->thing_count;
    while (t-- != 0) {
        Thing *thing = things[t];
        thing->update(thing);
    }

    Particle **particles = WORLD->particles;
    int p = WORLD->particle_count;
    while (p-- != 0) {
        Particle *particle = particles[p];
        if (particle->update(particle)) {
            WORLD->particle_count--;
            particles[p] = particles[WORLD->particle_count];
        }
    }

    WORLD->tick++;
}

void WorldFree() {
    WorldClear();
    Free(WORLD);
}
