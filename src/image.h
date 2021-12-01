/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef IMAGE_H
#define IMAGE_H

#include "mem.h"
#include "super.h"
#include "wad.h"

typedef struct Image Image;
typedef struct ImageFile ImageFile;

struct Image {
    i32 width;
    i32 height;
    u8 *pixels;
};

struct ImageFile {
    char *name;
    Image *image;
    // sprites;
};

Image *new_image();
void image_delete(Image *this);

ImageFile *read_image_file(String *content);
void image_file_delete(ImageFile *this);

#endif
