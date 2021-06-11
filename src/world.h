/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef WORLD_H
#define WORLD_H

#include <assert.h>
#include <float.h>
#include <math.h>

#include "math_util.h"
#include "mem.h"
#include "sector.h"
#include "set.h"
#include "sprite.h"

#define WORLD_SCALE 0.25f
#define WORLD_CELL_SHIFT 5

extern const float gravity;
extern const float wind_resistance;

extern unsigned int thing_unique_id;

enum thing_type { THING_TYPE_HERO, THING_TYPE_BARON, THING_TYPE_SCENERY };

typedef enum thing_type thing_type;

typedef struct world world;
typedef struct cell cell;
typedef struct thing thing;
typedef struct particle particle;
typedef struct decal decal;

struct world {
    char *name;
    thing **things;
    int thing_cap;
    int thing_count;
    thing **thing_sprites;
    int thing_sprites_cap;
    int thing_sprites_count;
    thing **thing_models;
    int thing_models_cap;
    int thing_models_count;
    particle **particles;
    int particle_cap;
    int particle_count;
    decal **decals;
    int decal_cap;
    int decal_count;
    sector **sectors;
    int sector_cap;
    int sector_count;
    cell *cells;
    int cell_columns;
    int cell_rows;
    int cell_count;
};

world *new_world();

void world_add_thing(world *this, thing *t);
void world_remove_thing(world *this, thing *t);
void world_add_particle(world *this, particle *t);
void world_remove_particle(world *this, particle *t);
void world_add_decal(world *this, decal *t);
void world_remove_decal(world *this, decal *t);
void world_add_sector(world *this, sector *s);
sector *world_find_sector(world *this, float x, float y);
void world_load_map(world *this);
void world_update(world *this);

struct cell {
    line **lines;
    int line_count;
    thing **things;
    int thing_cap;
    int thing_count;
    particle **particles;
    int particle_cap;
    int particle_count;
    decal **decals;
    int decal_cap;
    int decal_count;
};

void cell_add_line(cell *this, line *ld);
void cell_add_thing(cell *this, thing *t);
void cell_remove_thing(cell *this, thing *t);
void cell_add_particle(cell *this, particle *t);
void cell_remove_particle(cell *this, particle *t);
void cell_add_decal(cell *this, decal *t);
void cell_remove_decal(cell *this, decal *t);

struct thing {
    unsigned int id;
    thing_type type;
    world *map;
    sector *sec;
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
    sprite *sprite_data;
    void (*update)(void *);
};

void thing_initialize(thing *this, world *map, float x, float y, float r, float box, float height);
void thing_block_borders(thing *this);
void thing_nop_update(void *this);
void thing_standard_update(thing *this);

struct particle {
    float box;
    float height;
    float x;
    float y;
    float z;
    float dx;
    float dy;
    float dz;
    int texture;
    sprite *sprite_data;
    world *map;
    sector *sec;
    bool (*update)(void *);
};

particle *new_particle(world *map, float x, float y, float z, float box, float height);
void particle_hit_floor(particle *this);
void particle_hit_ceiling(particle *this);
bool particle_line_collision(particle *this, line *ld);
bool particle_map_collision(particle *this);

struct decal {
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

decal *new_decal(world *map);

#endif
