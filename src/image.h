/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef IMAGE_H
#define IMAGE_H

#include "mem.h"
#include "super.h"
#include "wad.h"

typedef struct Image Image;

struct Image {
    char name[16];
    u8 *pixels;
    // sprites
    i32 width;
    i32 height;
};

Image *NewImage(char name[16], u8 *pixels, i32 width, i32 height);

Image *ImageRead(String *string);

void ImageFree(Image *image);

#endif
