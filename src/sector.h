/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef SECTOR_H
#define SECTOR_H

#include <math.h>

#include "array.h"
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
    Vec *a;
    Vec *b;
    Wall *top;
    Wall *middle;
    Wall *bottom;
    Vec normal;
};

Line *new_line(Vec *a, Vec *b, int low, int mid, int top);
void line_set_sectors(Line *this, Sector *plus, Sector *minus);
MaybeVec line_intersect(Line *this, Line *with);

struct Wall {
    Vec *a;
    Vec *b;
    Vec normal;
    int texture;
    float floor;
    float ceiling;
    float u;
    float v;
    float s;
    float t;
};

Wall *new_wall(Vec *a, Vec *b, int texture);
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
    int floor_paint;
    int ceiling_paint;
    Triangle **triangles;
    int triangle_count;
    Sector **inside;
    int inside_count;
    Sector *outside;
};

Sector *new_sector(Vec **vecs, int vec_count, Line **lines, int line_count, float bottom, float floor, float ceiling, float top, int floor_paint, int ceiling_paint);
bool sector_contains(Sector *this, float x, float y);
Sector *sector_find(Sector *this, float x, float y);
bool sector_has_floor(Sector *this);
bool sector_has_ceiling(Sector *this);
void sector_inside_outside(Sector **sectors, int sector_count);

#endif
