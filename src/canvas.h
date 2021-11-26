/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef CANVAS_H
#define CANVAS_H

#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>

#include "hymn.h"
#include "mem.h"
#include "pie.h"
#include "vec.h"

typedef struct Canvas Canvas;

struct Canvas {
    i32 width;
    i32 height;
    u32 *pixels;
};

u32 rgb(u8 r, u8 g, u8 b);
i32 orient(i32 x0, i32 y0, i32 x1, i32 y1, i32 x2, i32 y2);

i32 abs32(i32 i);
i32 min32(i32 a, i32 b);
i32 max32(i32 a, i32 b);

Canvas *new_canvas(i32 width, i32 height);

void canvas_clear(Canvas *this);
void canvas_pixel(Canvas *this, u32 color, i32 x, i32 y);
void canvas_line(Canvas *this, u32 color, i32 x0, i32 y0, i32 x1, i32 y1);
void canvas_triangle(Canvas *this, u32 color, i32 x0, i32 y0, i32 x1, i32 y1, i32 x2, i32 y2);
void canvas_rectangle(Canvas *this, u32 color, i32 x0, i32 y0, i32 x1, i32 y1);
void canvas_project(Canvas *this, float *out, float *matrix, float *vec);
void canvas_rasterize(Canvas *this, float *a, float *b, float *c);

HymnValue canvas_rectangle_vm(Hymn *vm, int count, HymnValue *arguments);

#endif
