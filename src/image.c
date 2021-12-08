/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "image.h"

Image *NewImage(char name[16], u8 *pixels, i32 width, i32 height) {
    Image *image = Calloc(1, sizeof(Image));
    int n = 0;
    while (n < 16 && name[n] != '\0') {
        image->name[n] = name[n];
        n++;
    }
    image->name[n] = '\0';
    image->pixels = pixels;
    image->width = width;
    image->height = height;
    return image;
}

static void SpriteInit(Sprite *sprite, char name[16], Image *image, i32 left, i32 top, i32 right, i32 bottom) {
    int n = 0;
    while (n < 16 && name[n] != '\0') {
        sprite->name[n] = name[n];
        n++;
    }
    sprite->image = image;
    sprite->left = left;
    sprite->top = top;
    sprite->right = right;
    sprite->bottom = bottom;
    sprite->width = right - left;
    sprite->height = bottom - top;
}

Image *ImageRead(String *string) {
    MaybeWad maybe_wad = WadParse(string);
    if (maybe_wad.error != NULL) {
        fprintf(stderr, "%s\n", maybe_wad.error);
        exit(1);
    }
    Wad *wad = maybe_wad.wad;
    String *name = WadGetStringFromTable(wad, "image");
    int width = WadGetIntFromTable(wad, "columns");
    int height = WadGetIntFromTable(wad, "rows");
    int transparency = 0;
    if (WadHas(wad, "transparency")) {
        transparency = WadGetIntFromTable(wad, "transparency");
    }
    Array *data = WadGetArrayFromTable(wad, "pixels");
    u8 *pixels = Malloc(width * height);
    for (int h = 0; h < height; h++) {
        int row = h * height;
        for (int c = 0; c < width; c++) {
            int i = c + row;
            int p = WadAsInt(ArrayGet(data, i));
            pixels[i] = p == transparency ? UINT8_MAX : (u8)p;
        }
    }
    Image *image = NewImage(name, pixels, width, height);
    if (WadHas(wad, "sprites")) {
        Array *array = WadGetArrayFromTable(wad, "sprites");
        int sprite_count = ArraySize(array);
        Sprite *sprites = Calloc(sprite_count, sizeof(Sprite));
        for (int s = 0; s < sprite_count; s++) {
            Wad *sprite = ArrayGet(array, s);
            String *id = WadGetStringFromTable(sprite, "id");
            int left = WadGetIntFromTable(sprite, "left");
            int top = WadGetIntFromTable(sprite, "top");
            int right = WadGetIntFromTable(sprite, "right");
            int bottom = WadGetIntFromTable(sprite, "bottom");
            SpriteInit(&sprites[s], id, image, left, top, right, bottom);
        }
        image->sprites = sprites;
        image->sprite_count = sprite_count;
    }
    WadFree(wad);
    return image;
}

int SpriteIndex(Image *image, char *name) {
    for (int i = 0; i < image->sprite_count; i++) {
        if (strcmp(image->sprites[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

Sprite *SpriteSearch(Image *image, char *name) {
    int index = SpriteIndex(image, name);
    if (index == -1) {
        return NULL;
    }
    return &image->sprites[index];
}

void ImageFree(Image *image) {
    Free(image->pixels);
    Free(image->sprites);
    Free(image);
}
