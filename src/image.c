/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "image.h"

Image *new_image(i32 width, i32 height, u8 *pixels) {
    Image *image = safe_malloc(sizeof(Image));
    image->width = width;
    image->height = height;
    image->pixels = pixels;
    return image;
}

void image_delete(Image *this) {
    free(this->pixels);
    free(this);
}

ImageFile *read_image_file(String *content) {
    MaybeWad maybe_wad = wad_parse(content);
    if (maybe_wad.error != NULL) {
        fprintf(stderr, maybe_wad.error);
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
    WadArray *pixels = wad_get_array(wad_get_from_object(wad, "pixels"));
    u8 *out = safe_malloc(width * height);
    for (int h = 0; h < height; h++) {
        int row = h * height;
        for (int c = 0; c < width; c++) {
            int i = c + row;
            int p = wad_get_int(array_get(pixels, i));
            out[i] = p == transparency ? UINT8_MAX : (u8)p;
        }
    }
    Image *image = new_image(width, height, out);
    ImageFile *file = safe_calloc(1, sizeof(ImageFile));
    file->name = string_copy(name);
    file->image = image;
    wad_delete(wad);
    return file;
}

void image_file_delete(ImageFile *this) {
    free(this->name);
    free(this);
}
