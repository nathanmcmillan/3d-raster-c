/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef CANVAS_H
#define CANVAS_H

#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>

#include "hymn.h"
#include "image.h"
#include "mem.h"
#include "super.h"
#include "vec.h"

extern i32 SCREEN_WIDTH;
extern i32 SCREEN_HEIGHT;

extern u32 *PIXELS;

extern u32 PALETTE[UINT8_MAX];

u32 rgb(u8 r, u8 g, u8 b);
i32 orient(i32 x0, i32 y0, i32 x1, i32 y1, i32 x2, i32 y2);

i32 abs32(i32 i);
i32 min32(i32 a, i32 b);
i32 max32(i32 a, i32 b);
i32 clamp32(i32 a, i32 min, i32 max);

void CanvasInit(i32 width, i32 height);

void CanvasClear();
void CanvasFree();

void FontInit();

void CanvasPixel(u32 color, i32 x, i32 y);
void CanvasLine(u32 color, i32 x0, i32 y0, i32 x1, i32 y1);
void CanvasTriangle(u32 color, i32 x0, i32 y0, i32 x1, i32 y1, i32 x2, i32 y2);
void CanvasRectangle(u32 color, i32 x0, i32 y0, i32 x1, i32 y1);
void CanvasImage(Image *image, i32 x0, i32 y0);
void CanvasSubImage(Image *image, i32 x0, i32 y0, i32 left, i32 top, i32 right, i32 bottom);
void CanvasSprite(Sprite *sprite, i32 x0, i32 y0);
void CanvasText(Image *image, i32 x0, i32 y0, const char *text);
void ScreenSpace(float *out, float *matrix, float *vec);

HymnValue CanvasRectangleHymn(Hymn *hymn, int count, HymnValue *arguments);
HymnValue CanvasTextHymn(Hymn *hymn, int count, HymnValue *arguments);

#endif
