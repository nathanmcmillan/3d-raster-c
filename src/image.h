/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef IMAGE_H
#define IMAGE_H

#include "mem.h"
#include "super.h"
#include "wad.h"

typedef struct Image Image;
typedef struct Sprite Sprite;

struct Image {
    char name[16];
    u8 *pixels;
    Sprite *sprites;
    i32 width;
    i32 height;
    i16 sprite_count;
};

struct Sprite {
    char name[16];
    Image *image;
    i32 left;
    i32 top;
    i32 right;
    i32 bottom;
    i32 width;
    i32 height;
};

Image *NewImage(char name[16], u8 *pixels, i32 width, i32 height);
Image *ImageRead(String *string);
int ImageSpriteIndex(Image *image, char *name);
Sprite *ImageSpriteSearch(Image *image, char *name);
void ImageFree(Image *image);

#endif
