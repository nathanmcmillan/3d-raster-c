/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef SECTOR_H
#define SECTOR_H

#include <math.h>

#include "array.h"
#include "math_util.h"
#include "mem.h"
#include "set.h"
#include "vec.h"

#define LINE_NO_SIDE -1
#define SECTOR_NO_SURFACE -1

typedef struct Side Side;
typedef struct Line Line;
typedef struct Sector Sector;

struct Side {
    float x;
    float y;
    int top;
    int middle;
    int bottom;
};

struct Line {
    int id;
    Sector *front;
    Sector *back;
    Vec *a;
    Vec *b;
    Vec normal;
    Side side_front;
    Side side_back;
};

Line *NewLine(int id, Vec *a, Vec *b, Side front, Side back);

struct Sector {
    int id;
    Vec **vecs;
    int vec_count;
    Line **lines;
    int line_count;
    float bottom;
    float floor;
    float ceiling;
    float top;
    int floor_image;
    int ceiling_image;
    Sector **inside;
    int inside_count;
    Sector *outside;
    Sector **neighbors;
    int neighbor_count;
};

Sector *NewSector(int id, Line **lines, int line_count, float floor, float ceiling, int floor_image, int ceiling_image);
bool SectorContains(Sector *this, float x, float y);
Sector *SectorFind(Sector *this, float x, float y);
bool SectorHasFloor(Sector *this);
bool SectorHasCeiling(Sector *this);
void SectorInsideOutside(Sector **sectors, int sector_count);
void SectorNeighbors(Sector **sectors, int sector_count, Line **lines, int line_count);

#endif
