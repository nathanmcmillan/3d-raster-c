/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef ASSETS_H
#define ASSETS_H

#include "mem.h"
#include "paint.h"
#include "pie.h"
#include "table.h"

typedef struct Assets Assets;

struct Assets {
    Paint **paint;
    int paint_count;
    int paint_capacity;
    Table *paint_indices;
};

Assets *new_assets();

void assets_paint_save(Assets *this, char *name, Paint *paint);
int assets_paint_name_to_index(Assets *this, char *name);
Paint *assets_paint_get(Assets *this, int index);
Paint *assets_paint_find(Assets *this, char *name);

void assets_delete(Assets *this);

#endif
