#ifndef CANVAS_H
#define CANVAS_H

#include <inttypes.h>
#include <stdbool.h>

#include <lauxlib.h>
#include <lua.h>

#include "pie.h"

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

void canvas_clear(Canvas *this);
void canvas_line(Canvas *this, u32 color, i32 x0, i32 y0, i32 x1, i32 y1);
void canvas_triangle(Canvas *this, u32 color, i32 x0, i32 y0, i32 x1, i32 y1, i32 x2, i32 y2);
void canvas_rect(Canvas *this, u32 color, i32 x0, i32 y0, i32 x1, i32 y1);

int vm_canvas_rect(lua_State *vm);

#endif
