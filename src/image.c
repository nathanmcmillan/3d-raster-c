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

Image *ImageRead(String *string) {
    MaybeWad maybe_wad = wad_parse(string);
    if (maybe_wad.error != NULL) {
        fprintf(stderr, "%s\n", maybe_wad.error);
        exit(1);
    }
    Wad *wad = maybe_wad.wad;
    String *name = wad_get_string(wad_get_from_object(wad, "paint"));
    i32 width = (i32)wad_get_int(wad_get_from_object(wad, "width"));
    i32 height = (i32)wad_get_int(wad_get_from_object(wad, "height"));
    int transparency = 0;
    if (wad_has(wad, "transparency")) {
        transparency = wad_get_int(wad_get_from_object(wad, "transparency"));
    }
    WadArray *data = wad_get_array(wad_get_from_object(wad, "pixels"));
    u8 *pixels = Malloc(width * height);
    for (int h = 0; h < height; h++) {
        int row = h * height;
        for (int c = 0; c < width; c++) {
            int i = c + row;
            int p = wad_get_int(array_get(data, i));
            pixels[i] = p == transparency ? UINT8_MAX : (u8)p;
        }
    }
    Image *image = NewImage(name, pixels, width, height);
    wad_delete(wad);
    return image;
}

void ImageFree(Image *image) {
    Free(image->pixels);
    Free(image);
}
