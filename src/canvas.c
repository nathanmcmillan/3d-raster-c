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

Canvas *new_canvas(i32 width, i32 height) {
    Canvas *this = safe_calloc(1, sizeof(Canvas));
    this->width = width;
    this->height = height;
    this->pixels = safe_calloc(width * height, sizeof(u32));
    this->depth = safe_calloc(width * height, sizeof(float));
    return this;
}

void canvas_clear_color(Canvas *this) {
    memset(this->pixels, 0, this->width * this->height * sizeof(u32));
}

void canvas_clear_depth(Canvas *this) {
    memset(this->depth, INT_MAX, this->width * this->height * sizeof(float));
}

void canvas_pixel(Canvas *this, u32 color, i32 x, i32 y) {
    i32 width = this->width;
    if (x >= 0 && y >= 0 && x < width && y < this->height) {
        this->pixels[x + y * width] = color;
    }
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

void canvas_project(Canvas *this, float *out, float *matrix, float *vec) {

    float x = vec[0] * matrix[0] + vec[1] * matrix[4] + vec[2] * matrix[8] + matrix[12];
    float y = vec[0] * matrix[1] + vec[1] * matrix[5] + vec[2] * matrix[9] + matrix[13];
    float z = vec[0] * matrix[2] + vec[1] * matrix[6] + vec[2] * matrix[10] + matrix[14];
    float w = vec[0] * matrix[3] + vec[1] * matrix[7] + vec[2] * matrix[11] + matrix[15];

    if (w != 1.0f) {
        x /= w;
        y /= w;
        z /= w;
    }

    x = x * (float)this->width + 0.5f * (float)this->width;
    y = -y * (float)this->height + 0.5f * (float)this->height;

    out[0] = x;
    out[1] = y;
    out[2] = z;
    out[3] = w;
}

static void rasterize(Canvas *this, u32 color, i32 x0, i32 y0, i32 x1, i32 y1, i32 x2, i32 y2) {
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

void canvas_rasterize(Canvas *this, float *a, float *b, float *c) {
    i32 width = this->width;
    i32 height = this->height;
    u32 *pixels = this->pixels;
    float *z = this->depth;

    i32 min_x = max32(min32(min32((i32)a[0], (i32)b[0]), (i32)c[0]), 0);
    i32 min_y = max32(min32(min32((i32)a[1], (i32)b[1]), (i32)c[1]), 0);
    i32 max_x = min32(max32(max32((i32)a[0], (i32)b[0]), (i32)c[0]), width - 1);
    i32 max_y = min32(max32(max32((i32)a[1], (i32)b[1]), (i32)c[1]), height - 1);

    for (i32 y = min_y; y < max_y; y++) {
        for (i32 x = min_x; x < max_x; x++) {
            i32 w0 = orient((i32)a[0], (i32)a[1], (i32)b[0], (i32)b[1], x, y);
            i32 w1 = orient((i32)c[0], (i32)c[1], (i32)a[0], (i32)a[1], x, y);
            i32 w2 = orient((i32)a[0], (i32)a[1], (i32)b[0], (i32)b[1], x, y);

            float depth = 0.0f;
            if (depth > z[0]) {
                continue;
            }

            i32 color = rgb(255, 0, 0);

            if (w0 >= 0 and w1 >= 0 and w2 >= 0) {
                i32 i = x + y * width;
                pixels[i] = color;
                z[i] = depth;
            }
        }
    }
}

char *canvas_rect_vm(Hymn *vm) {
    Canvas *canvas = hymn_pointer(vm, 0);
    u32 color = hymn_u32(vm, 1);
    i32 x0 = hymn_i32(vm, 2);
    i32 y0 = hymn_i32(vm, 3);
    i32 x1 = hymn_i32(vm, 4);
    i32 y1 = hymn_i32(vm, 5);
    canvas_rect(canvas, color, x0, y0, x1, y1);
    return NULL;
}
