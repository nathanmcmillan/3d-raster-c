/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef WORLD_H
#define WORLD_H

#include <float.h>
#include <math.h>

#include "array.h"
#include "image.h"
#include "math_util.h"
#include "mem.h"
#include "sector.h"
#include "set.h"
#include "super.h"
#include "world.h"

#define WORLD_SCALE 0.25f
#define WORLD_CELL_SHIFT 5

#define GRAVITY 0.028f
#define RESISTANCE 0.88f

enum ThingType {
    THING_TYPE_HERO,
    THING_TYPE_BARON,
    THING_TYPE_SCENERY,
};

typedef struct World World;
typedef struct Cell Cell;
typedef struct Thing Thing;
typedef struct Particle Particle;
typedef struct Decal Decal;

struct World {
    char *name;
    Line **lines;
    int line_count;
    Thing **things;
    int thing_cap;
    int thing_count;
    Particle **particles;
    int particle_cap;
    int particle_count;
    Decal **decals;
    int decal_cap;
    int decal_count;
    Sector **sectors;
    int sector_cap;
    int sector_count;
    Cell *cells;
    int columns;
    int rows;
    int cell_count;
    u32 tick;
    u32 *sectors_visited;
    u32 *lines_visited;
};

World *new_world();

int world_cell(float f);
float world_float_cell(float f);

void WorldClear(World *this);
void world_add_thing(World *this, Thing *t);
void world_remove_thing(World *this, Thing *t);
void world_add_particle(World *this, Particle *t);
void world_remove_particle(World *this, Particle *t);
void world_add_decal(World *this, Decal *t);
void world_remove_decal(World *this, Decal *t);
void world_add_sector(World *this, Sector *s);
Sector *world_find_sector(World *this, float x, float y);
void WorldBuild(World *this, Line **lines, int lint_count);
void WorldUpdate(World *this);

struct Cell {
    Line **lines;
    int line_count;
    Thing **things;
    int thing_cap;
    int thing_count;
    Particle **particles;
    int particle_cap;
    int particle_count;
    Decal **decals;
    int decal_cap;
    int decal_count;
};

void cell_add_line(Cell *this, Line *ld);
void cell_add_thing(Cell *this, Thing *t);
void cell_remove_thing(Cell *this, Thing *t);
void cell_add_particle(Cell *this, Particle *t);
void cell_remove_particle(Cell *this, Particle *t);
void cell_add_decal(Cell *this, Decal *t);
void cell_remove_decal(Cell *this, Decal *t);

struct Thing {
    unsigned int id;
    enum ThingType type;
    World *map;
    Sector *sec;
    int health;
    float box;
    float height;
    float speed;
    float x;
    float y;
    float z;
    float dx;
    float dy;
    float dz;
    float previous_x;
    float previous_z;
    float rotation;
    float rotation_target;
    bool ground;
    int c_min;
    int r_min;
    int c_max;
    int r_max;
    int sprite_id;
    Sprite *sprite_data;
    void (*update)(void *);
};

void thing_initialize(Thing *this, World *map, float x, float y, float r, float box, float height);
void thing_block_borders(Thing *this);
void thing_nop_update(void *this);
void thing_standard_update(Thing *this);

struct Particle {
    float box;
    float height;
    float x;
    float y;
    float z;
    float dx;
    float dy;
    float dz;
    int texture;
    Sprite *sprite_data;
    World *map;
    Sector *sec;
    bool (*update)(void *);
};

Particle *new_particle(World *map, float x, float y, float z, float box, float height);
void particle_hit_floor(Particle *this);
void particle_hit_ceiling(Particle *this);
bool particle_line_collision(Particle *this, Line *ld);
bool particle_map_collision(Particle *this);

struct Decal {
    float x1;
    float y1;
    float z1;
    float u1;
    float v1;
    float x2;
    float y2;
    float z2;
    float u2;
    float v2;
    float x3;
    float y3;
    float z3;
    float u3;
    float v3;
    float x4;
    float y4;
    float z4;
    float u4;
    float v4;
    float nx;
    float ny;
    float nz;
    int texture;
};

Decal *new_decal(World *map);

void world_delete(World *world);

#endif
