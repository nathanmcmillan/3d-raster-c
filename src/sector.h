/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef SECTOR_H
#define SECTOR_H

#include <float.h>
#include <math.h>

#include "array.h"
#include "image.h"
#include "math_util.h"
#include "mem.h"
#include "set.h"
#include "super.h"
#include "vec.h"

#define LINE_NO_SIDE -1
#define SECTOR_NO_SURFACE -1

#define GRAVITY 0.028f
#define RESISTANCE 0.88f

#define WORLD_SCALE 0.25f
#define WORLD_CELL_SHIFT 5

typedef struct Side Side;
typedef struct Line Line;
typedef struct Thing Thing;
typedef struct Particle Particle;
typedef struct Decal Decal;
typedef struct Cell Cell;
typedef struct Sector Sector;
typedef struct World World;

extern World *WORLD;

extern Cell *CELLS;
extern int CELL_COLUMNS;
extern int CELL_ROWS;

struct Side {
    float x;
    float y;
    int top;
    int middle;
    int bottom;
};

struct Line {
    int id;
    u32 visited;
    Sector *front;
    Sector *back;
    Vec *a;
    Vec *b;
    Vec normal;
    Side side_front;
    Side side_back;
};

Line *NewLine(int id, Vec *a, Vec *b, Side front, Side back);

enum ThingType {
    THING_TYPE_HERO,
    THING_TYPE_BARON,
    THING_TYPE_SCENERY,
};

struct Thing {
    int id;
    u32 visited;
    enum ThingType type;
    Sector *sector;
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
    int min_c;
    int min_r;
    int max_c;
    int max_r;
    int sprite_id;
    Sprite *sprite_data;
    void (*update)(void *);
};

Thing *NewThing(float x, float y, float z);

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

struct Cell {
    Line **lines;
    int line_count;
    Thing **things;
    int thing_capacity;
    int thing_count;
    Particle **particles;
    int particle_capacity;
    int particle_count;
};

void CellAddLine(Cell *this, Line *line);
void CellAddThing(Cell *this, Thing *thing);
void CellRemoveThing(Cell *this, Thing *thing);
void CellAddParticle(Cell *this, Particle *particle);
void CellRemoveParticle(Cell *this, Particle *particle);

struct Sector {
    int id;
    u32 visited;
    Vec **vecs;
    int vec_count;
    Line **lines;
    int line_count;
    Thing **things;
    int thing_capacity;
    int thing_count;
    float floor;
    float ceiling;
    int floor_image;
    int ceiling_image;
    Sector **inside;
    int inside_count;
    Sector *outside;
    Sector **neighbors;
    int neighbor_count;
};

void SectorsInit();
void SectorsFree();

Sector *NewSector(int id, Line **lines, int line_count, float floor, float ceiling, int floor_image, int ceiling_image);
bool SectorContains(Sector *sector, float x, float z);
Sector *SectorFind(Sector *sector, float x, float z);
Sector *SectorSearchFor(Sector *sector, float x, float z);
bool SectorHasFloor(Sector *sector);
bool SectorHasCeiling(Sector *sector);
void SectorInsideOutside(Sector **sectors, int sector_count);
void SectorNeighbors(Sector **sectors, int sector_count, Line **lines, int line_count);
void SectorAddThing(Sector *sector, Thing *thing);
void SectorRemoveThing(Sector *sector, Thing *thing);

struct World {
    char *name;
    Line **lines;
    int line_count;
    Thing **things;
    int thing_capacity;
    int thing_count;
    Particle **particles;
    int particle_capacity;
    int particle_count;
    Decal **decals;
    int decal_capacity;
    int decal_count;
    Sector **sectors;
    int sector_capacity;
    int sector_count;
    u32 tick;
};

void WorldInit();

int WorldCell(float f);
float WorldFloatCell(float f);

void WorldClear();
void WorldAddThing(Thing *thing);
void WorldRemoveThing(Thing *thing);
void WorldAddParticle(Particle *particle);
void WorldRemoveParticle(Particle *particle);
void WorldAddDecal(Decal *decal);
void WorldRemoveDecal(Decal *decal);
void WorldAddSector(Sector *sector);
Sector *WorldFindSector(float x, float y);
void WorldBuild(Line **lines, int line_count);
Thing *WorldSpawnEntity(String *id, float x, float y, float z);
void WorldUpdate();

void WorldFree();

#endif
