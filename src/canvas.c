/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "canvas.h"

i32 SCREEN_WIDTH = 0;
i32 SCREEN_HEIGHT = 0;

u32 *PIXELS = NULL;

u32 BLACK = 0xff000000;
u32 STORM = 0xff1d2b53;
u32 WINE = 0xff7e2553;
u32 MOSS = 0xff008751;
u32 TAN = 0xffab5236;
u32 SLATE = 0xff5f574f;
u32 SILVER = 0xffc2c3c7;
u32 WHITE = 0xfffff1e8;
u32 EMBER = 0xffff004d;
u32 ORANGE = 0xffffa300;
u32 LEMON = 0xffffec27;
u32 LIME = 0xff00e436;
u32 SKY = 0xff29adff;
u32 DUSK = 0xff83769C;
u32 PINK = 0xffff77a8;
u32 PEACH = 0xffffccaa;

u32 COCOA = 0xff291814;
u32 MIDNIGHT = 0xff111d35;
u32 PORT = 0xff422136;
u32 SEA = 0xff125359;
u32 LEATHER = 0xff742f29;
u32 CHARCOAL = 0xff49333b;
u32 OLIVE = 0xffa28879;
u32 SAND = 0xfff3ef7d;
u32 CRIMSON = 0xffbe1250;
u32 AMBER = 0xffff6c24;
u32 TEA = 0xffa8e72e;
u32 JADE = 0xff00b543;
u32 DENIM = 0xff065ab5;
u32 AUGERGINE = 0xff754665;
u32 SALMON = 0xffff6e59;
u32 CORAL = 0xffff9d81;

u32 PALETTE[UINT8_MAX] = {0};

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

i32 clamp32(i32 a, i32 min, i32 max) {
    return min32(max32(a, min), max);
}

void PaletteDefault() {
    PALETTE[0] = BLACK;
    PALETTE[1] = STORM;
    PALETTE[2] = WINE;
    PALETTE[3] = MOSS;
    PALETTE[4] = TAN;
    PALETTE[5] = SLATE;
    PALETTE[6] = SILVER;
    PALETTE[7] = WHITE;
    PALETTE[8] = EMBER;
    PALETTE[9] = ORANGE;
    PALETTE[10] = LEMON;
    PALETTE[11] = LIME;
    PALETTE[12] = SKY;
    PALETTE[13] = DUSK;
    PALETTE[14] = PINK;
    PALETTE[15] = PEACH;

    PALETTE[16] = COCOA;
    PALETTE[17] = MIDNIGHT;
    PALETTE[18] = PORT;
    PALETTE[19] = SEA;
    PALETTE[20] = LEATHER;
    PALETTE[21] = CHARCOAL;
    PALETTE[22] = OLIVE;
    PALETTE[23] = SAND;
    PALETTE[23] = CRIMSON;
    PALETTE[24] = AMBER;
    PALETTE[25] = TEA;
    PALETTE[26] = JADE;
    PALETTE[27] = DENIM;
    PALETTE[28] = AUGERGINE;
    PALETTE[29] = SALMON;
    PALETTE[30] = CORAL;
}

void CanvasInit(i32 width, i32 height) {
    SCREEN_WIDTH = width;
    SCREEN_HEIGHT = height;
    Free(PIXELS);
    PIXELS = Calloc(width * height, sizeof(u32));
    PaletteDefault();
}

void CanvasClear() {
    memset(PIXELS, 0, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(u32));
}

void CanvasFree() {
    Free(PIXELS);
}

void CavnasPixel(u32 color, i32 x, i32 y) {
    if (x >= 0 && y >= 0 && x < SCREEN_WIDTH && y < SCREEN_HEIGHT) {
        PIXELS[x + y * SCREEN_WIDTH] = color;
    }
}

void CanvasLine(u32 color, i32 x0, i32 y0, i32 x1, i32 y1) {
    i32 dx = abs32(x1 - x0);
    i32 sx = (x0 < x1) ? 1 : -1;
    i32 dy = abs32(y1 - y0);
    i32 sy = (y0 < y1) ? 1 : -1;
    i32 err = ((dx > dy) ? dx : -dy) / 2;
    i32 err2;
    i32 x = x0;
    i32 y = y0;
    while (true) {
        if (x < 0 || y < 0) {
            return;
        }
        i32 px = x;
        i32 py = y;
        if (px >= SCREEN_WIDTH || py >= SCREEN_HEIGHT) {
            return;
        }
        PIXELS[px + py * SCREEN_WIDTH] = color;
        if (x == x1 && y == y1) {
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

void CanvasTriangle(u32 color, i32 x0, i32 y0, i32 x1, i32 y1, i32 x2, i32 y2) {
    i32 min_x = max32(min32(min32(x0, x1), x2), 0);
    i32 min_y = max32(min32(min32(y0, y1), y2), 0);
    i32 max_x = min32(max32(max32(x0, x1), x2), SCREEN_WIDTH - 1);
    i32 max_y = min32(max32(max32(y0, y1), y2), SCREEN_HEIGHT - 1);
    for (i32 y = min_y; y < max_y; y++) {
        for (i32 x = min_x; x < max_x; x++) {
            i32 w0 = orient(x1, y1, x2, y2, x, y);
            i32 w1 = orient(x2, y2, x0, y0, x, y);
            i32 w2 = orient(x0, y0, x1, y1, x, y);
            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                PIXELS[x + y * SCREEN_WIDTH] = color;
            }
        }
    }
}

void CanvasRectangle(u32 color, i32 x0, i32 y0, i32 x1, i32 y1) {
    i32 min_x = max32(min32(x0, x1), 0);
    i32 min_y = max32(min32(y0, y1), 0);
    i32 max_x = min32(max32(x0, x1), SCREEN_WIDTH - 1);
    i32 max_y = min32(max32(y0, y1), SCREEN_HEIGHT - 1);

    for (i32 y = min_y; y < max_y; y++) {
        for (i32 x = min_x; x < max_x; x++) {
            PIXELS[x + y * SCREEN_WIDTH] = color;
        }
    }
}

void CanvasImage(Image *image, i32 x0, i32 y0) {
    u8 *source = image->pixels;
    i32 span = image->width;

    i32 columns = span;
    i32 rows = image->height;

    if (x0 < 0) {
        source -= x0;
        columns -= x0;
        x0 = 0;
    }

    if (y0 < 0) {
        source -= y0 * span;
        rows -= y0;
        y0 = 0;
    }

    if (x0 + columns > SCREEN_WIDTH - 1) {
        columns = SCREEN_WIDTH - 1 - x0;
    }

    if (y0 + rows > SCREEN_HEIGHT - 1) {
        rows = SCREEN_HEIGHT - 1 - y0;
    }

    u32 *destination = &PIXELS[x0 + y0 * SCREEN_WIDTH];

    for (i32 r = 0; r < rows; r++) {
        u8 *slice = source;
        u32 *color = destination;
        for (i32 c = 0; c < columns; c++) {
            u8 pixel = *slice;
            if (pixel != UINT8_MAX)
                *color = PALETTE[pixel];
            slice++;
            color++;
        }
        source += span;
        destination += SCREEN_WIDTH;
    }
}

void CanvasSubImage(Image *image, i32 x0, i32 y0, i32 left, i32 top, i32 right, i32 bottom) {
    u8 *source = image->pixels;
    i32 span = image->width;

    i32 columns = right - left;
    i32 rows = bottom - top;

    source += left + top * span;

    if (x0 < 0) {
        source -= x0;
        columns -= x0;
        x0 = 0;
    }

    if (y0 < 0) {
        source -= y0 * span;
        rows -= y0;
        y0 = 0;
    }

    if (x0 + columns > SCREEN_WIDTH - 1) {
        columns = SCREEN_WIDTH - 1 - x0;
    }

    if (y0 + rows > SCREEN_HEIGHT - 1) {
        rows = SCREEN_HEIGHT - 1 - y0;
    }

    u32 *destination = &PIXELS[x0 + y0 * SCREEN_WIDTH];

    for (i32 r = 0; r < rows; r++) {
        u8 *slice = source;
        u32 *color = destination;
        for (i32 c = 0; c < columns; c++) {
            u8 pixel = *slice;
            if (pixel != UINT8_MAX)
                *color = PALETTE[pixel];
            slice++;
            color++;
        }
        source += span;
        destination += SCREEN_WIDTH;
    }
}

void CanvasSprite(Sprite *sprite, i32 x0, i32 y0) {
    u8 *source = sprite->image->pixels;
    i32 span = sprite->image->width;

    i32 columns = sprite->width;
    i32 rows = sprite->height;

    source += sprite->left + sprite->top * span;

    if (x0 < 0) {
        source -= x0;
        columns -= x0;
        x0 = 0;
    }

    if (y0 < 0) {
        source -= y0 * span;
        rows -= y0;
        y0 = 0;
    }

    if (x0 + columns > SCREEN_WIDTH - 1) {
        columns = SCREEN_WIDTH - 1 - x0;
    }

    if (y0 + rows > SCREEN_HEIGHT - 1) {
        rows = SCREEN_HEIGHT - 1 - y0;
    }

    u32 *destination = &PIXELS[x0 + y0 * SCREEN_WIDTH];

    for (i32 r = 0; r < rows; r++) {
        u8 *slice = source;
        u32 *color = destination;
        for (i32 c = 0; c < columns; c++) {
            u8 pixel = *slice;
            if (pixel != UINT8_MAX)
                *color = PALETTE[pixel];
            slice++;
            color++;
        }
        source += span;
        destination += SCREEN_WIDTH;
    }
}

static const char *FONT = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";

typedef struct Font Font;

struct Font {
    i32 width;
    i32 height;
    i32 base;
    i32 columns;
};

static Font NewFont(i32 width, i32 height, i32 base) {
    Font font = {0};
    font.width = width;
    font.height = height;
    font.base = base;
    font.columns = 128 / width;
    return font;
}

static Font TIC_FONT = {0};

void FontInit() {
    TIC_FONT = NewFont(6, 6, 5);
}

void CanvasText(Image *image, i32 x0, i32 y0, const char *text) {
    Font *font = &TIC_FONT;
    i32 x = x0;
    i32 y = y0;
    i32 width = font->width;
    i32 height = font->height;
    i32 columns = font->columns;
    usize length = strlen(text);
    for (usize i = 0; i < length; i++) {
        char c = text[i];
        if (c == ' ') {
            x += width;
        } else if (c == '\n') {
            x = x0;
            y += height;
        }
        char *pointer = strchr(FONT, c);
        if (pointer == NULL)
            continue;
        i32 index = pointer - FONT;
        i32 left = (i32)floorf(index % columns) * width;
        i32 top = (i32)floorf(index / columns) * height;
        i32 right = left + width;
        i32 bottom = top + height;
        CanvasSubImage(image, x, y, left, top, right, bottom);
        x += width;
    }
}

void ScreenSpace(float *out, float *matrix, float *vec) {
    float x = vec[0] * matrix[0] + vec[1] * matrix[4] + vec[2] * matrix[8] + matrix[12];
    float y = vec[0] * matrix[1] + vec[1] * matrix[5] + vec[2] * matrix[9] + matrix[13];
    float z = vec[0] * matrix[2] + vec[1] * matrix[6] + vec[2] * matrix[10] + matrix[14];
    float w = vec[0] * matrix[3] + vec[1] * matrix[7] + vec[2] * matrix[11] + matrix[15];

    if (w != 1.0f) {
        x /= w;
        y /= w;
        z /= w;
    }

    x = x * (float)SCREEN_WIDTH + 0.5f * (float)SCREEN_WIDTH;
    y = -y * (float)SCREEN_HEIGHT + 0.5f * (float)SCREEN_HEIGHT;

    out[0] = x;
    out[1] = y;
    out[2] = z;
    out[3] = w;
}

HymnValue CanvasRectangleHymn(Hymn *hymn, int count, HymnValue *arguments) {
    (void)hymn;
    if (count != 6)
        return hymn_new_none();
    i64 color = hymn_as_int(arguments[1]);
    i64 x0 = hymn_as_int(arguments[2]);
    i64 y0 = hymn_as_int(arguments[3]);
    i64 x1 = hymn_as_int(arguments[4]);
    i64 y1 = hymn_as_int(arguments[5]);
    CanvasRectangle((u32)color, (i32)x0, (i32)y0, (i32)x1, (i32)y1);
    return hymn_new_none();
}

HymnValue CanvasTextHymn(Hymn *hymn, int count, HymnValue *arguments) {
    (void)hymn;
    if (count != 4)
        return hymn_new_none();
    Image *image = hymn_as_pointer(arguments[0]);
    i64 x0 = hymn_as_int(arguments[1]);
    i64 y0 = hymn_as_int(arguments[2]);
    char *text = hymn_as_string(arguments[3]);
    CanvasText(image, (i32)x0, (i32)y0, text);
    return hymn_new_none();
}
