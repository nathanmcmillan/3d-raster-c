/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "resources.h"

static Image **IMAGES = NULL;
static int IMAGE_COUNT = 0;
static int IMAGE_CAPACITY = 0;

void ResourceAddImage(Image *image) {
    if (IMAGE_CAPACITY == 0) {
        IMAGES = Malloc(sizeof(Image *));
        IMAGE_CAPACITY = 1;
    } else if (IMAGE_COUNT == IMAGE_CAPACITY) {
        IMAGE_CAPACITY += 8;
        IMAGES = Realloc(IMAGES, IMAGE_CAPACITY * sizeof(Image *));
    }
    IMAGES[IMAGE_COUNT] = image;
    IMAGE_COUNT++;

    for (int s = 0; s < image->sprite_count; s++) {
        Sprite *sprite = &image->sprites[s];
        i32 width = sprite->width;
        i32 height = sprite->height;
        u8 *source = sprite->image->pixels;
        i32 bound = sprite->image->width;
        u8 *pixels = Malloc(width * height * sizeof(u8));
        i32 i = 0;
        for (i32 y = sprite->top; y < sprite->bottom; y++) {
            for (i32 x = sprite->left; x < sprite->right; x++) {
                pixels[i++] = source[x + y * bound];
            }
        }
        Image *sprite_image = NewImage(sprite->name, pixels, width, height);
        ResourceAddImage(sprite_image);
    }
}

Image *ResourceImage(int index) {
    return IMAGES[index];
}

int ResourceImageIndex(char *name) {
    for (int i = 0; i < IMAGE_COUNT; i++) {
        if (strcmp(IMAGES[i]->name, name) == 0) {
            return i;
        }
    }
    return -1;
}

Image *ResourceImageSearch(char *name) {
    int index = ResourceImageIndex(name);
    if (index == -1) {
        return NULL;
    }
    return IMAGES[index];
}

void ResourceFree() {
    Free(IMAGES);
}
