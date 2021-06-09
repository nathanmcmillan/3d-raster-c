/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "canvas.h"

u32 rgb(u8 r, u8 g, u8 b) {
    return ((u32)r << 16) | ((u32)g << 8) | (u32)b;
}

i32 orient(i32 x0, i32 y0, i32 x1, i32 y1, i32 x2, i32 y2) {
    return (x1 - x0) * (y2 - y0) - (y1 - y0) * (x2 - x0);
}

i32 abs32(i32 i) {
    return (i < 0) ? -i : i;
}

i32 min32(i32 a, i32 b) {
    return (a < b) ? a : b;
}

i32 max32(i32 a, i32 b) {
    return (a > b) ? a : b;
}

void canvas_clear(Canvas *this) {
    memset(this->pixels, 0, this->width * this->height * sizeof(u32));
}

void canvas_line(Canvas *this, u32 color, i32 x0, i32 y0, i32 x1, i32 y1) {
    i32 width = this->width;
    i32 height = this->height;
    u32 *pixels = this->pixels;

    i32 dx = abs32(x1 - x0);
    i32 sx = (x0 < x1) ? 1 : -1;
    i32 dy = abs32(y1 - y0);
    i32 sy = (y0 < y1) ? 1 : -1;
    i32 err = ((dx > dy) ? dx : -dy) / 2;
    i32 err2;
    i32 x = x0;
    i32 y = y0;

    while (true) {
        if (x < 0 or y < 0) {
            return;
        }
        i32 px = x;
        i32 py = y;
        if (px >= width or py >= height) {
            return;
        }
        pixels[px + py * width] = color;
        if (x == x1 and y == y1) {
            break;
        }
        err2 = err;
        if (err2 > -dx) {
            err -= dy;
            x += sx;
        }
        if (err2 < dy) {
            err += dx;
            y += sy;
        }
    }
}

void canvas_triangle(Canvas *this, u32 color, i32 x0, i32 y0, i32 x1, i32 y1, i32 x2, i32 y2) {
    i32 width = this->width;
    i32 height = this->height;
    u32 *pixels = this->pixels;

    i32 min_x = max32(min32(min32(x0, x1), x2), 0);
    i32 min_y = max32(min32(min32(y0, y1), y2), 0);
    i32 max_x = min32(max32(max32(x0, x1), x2), width - 1);
    i32 max_y = min32(max32(max32(y0, y1), y2), height - 1);

    for (i32 y = min_y; y < max_y; y++) {
        for (i32 x = min_x; x < max_x; x++) {
            i32 w0 = orient(x1, y1, x2, y2, x, y);
            i32 w1 = orient(x2, y2, x0, y0, x, y);
            i32 w2 = orient(x0, y0, x1, y1, x, y);
            if (w0 >= 0 and w1 >= 0 and w2 >= 0) {
                pixels[x + y * width] = color;
            }
        }
    }
}

void canvas_rect(Canvas *this, u32 color, i32 x0, i32 y0, i32 x1, i32 y1) {
    i32 width = this->width;
    i32 height = this->height;
    u32 *pixels = this->pixels;

    i32 min_x = max32(min32(x0, x1), 0);
    i32 min_y = max32(min32(y0, y1), 0);
    i32 max_x = min32(max32(x0, x1), width - 1);
    i32 max_y = min32(max32(y0, y1), height - 1);

    for (i32 y = min_y; y < max_y; y++) {
        for (i32 x = min_x; x < max_x; x++) {
            pixels[x + y * width] = color;
        }
    }
}

int vm_canvas_rect(lua_State *vm) {
    Canvas *canvas = lua_touserdata(vm, 1);
    u32 color = (u32)lua_tonumber(vm, 2);
    i32 x0 = (i32)luaL_checknumber(vm, 3);
    i32 y0 = (i32)luaL_checknumber(vm, 4);
    i32 x1 = (i32)luaL_checknumber(vm, 5);
    i32 y1 = (i32)luaL_checknumber(vm, 6);
    canvas_rect(canvas, color, x0, y0, x1, y1);
    return 0;
}
