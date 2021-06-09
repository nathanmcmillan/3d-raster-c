#ifndef TEXTURE_H
#define TEXTURE_H

#include "mem.h"
#include "pie.h"

typedef struct Texture Texture;

struct Texture {
    i32 width;
    i32 height;
    u8 *pixels;
};

Texture *new_texture();

#endif
