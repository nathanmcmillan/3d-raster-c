/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef SECTOR_H
#define SECTOR_H

#include <math.h>

#include "math_util.h"
#include "mem.h"
#include "triangle.h"
#include "vec.h"

#define LINE_NO_WALL -1
#define SECTOR_NO_SURFACE -1

extern unsigned int sector_unique_id;

typedef struct Line Line;
typedef struct Wall Wall;
typedef struct Sector Sector;

struct Line {
    Sector *plus;
    Sector *minus;
    Vec va;
    Vec vb;
    Wall *top;
    Wall *middle;
    Wall *bottom;
    Vec normal;
};

Line *line_init(Vec va, Vec vb, int low, int mid, int top);
void line_set_sectors(Line *this, Sector *plus, Sector *minus);
VecOk line_intersect(Line *this, Line *with);

struct Wall {
    Line *ld;
    Vec va;
    Vec vb;
    float floor;
    float ceiling;
    int texture;
    float u;
    float v;
    float s;
    float t;
};

Wall *wall_init(Line *ld, Vec va, Vec vb, int texture);
void wall_set(Wall *this, float floor, float ceiling, float u, float v, float s, float t);

struct Sector {
    unsigned int id;
    Vec **vecs;
    int vec_count;
    Line **lines;
    int line_count;
    float bottom;
    float floor;
    float ceiling;
    float top;
    int floor_texture;
    int ceiling_texture;
    Triangle **triangles;
    int triangle_count;
    Sector **inside;
    int inside_count;
    Sector *outside;
};

Sector *sector_init(Vec **vecs, int vec_count, Line **lines, int line_count, float bottom, float floor, float ceiling, float top, int floor_texture, int ceiling_texture);
bool sector_contains(Sector *this, float x, float y);
Sector *sector_find(Sector *this, float x, float y);
bool sector_has_floor(Sector *this);
bool sector_has_ceiling(Sector *this);

#endif
