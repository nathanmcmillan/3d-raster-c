/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef RESOURCES_H
#define RESOURCES_H

#include "image.h"
#include "mem.h"
#include "string_util.h"
#include "super.h"
#include "table.h"
#include "wad.h"

typedef struct Resources Resources;

struct Resources {
    Image **images;
    int image_count;
    int image_capacity;
    Table *image_table;
};

Resources *new_resources();

void resources_add_image(Resources *this, char *name, Image *paint);
int resources_image_name_to_index(Resources *this, char *name);
Image *resources_image_get(Resources *this, int index);
Image *resources_image_find(Resources *this, char *name);

void resources_delete(Resources *this);

#endif
