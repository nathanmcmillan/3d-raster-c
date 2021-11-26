/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "assets.h"

Assets *new_assets() {
    Assets *assets = safe_calloc(1, sizeof(Assets));
    assets->paint_indices = new_string_table();
    return assets;
}

void assets_paint_save(Assets *this, char *name, Image *paint) {

    if (this->paint_capacity == 0) {
        this->paint = safe_malloc(sizeof(Image *));
        this->paint[0] = paint;
        this->paint_capacity = 1;
        this->paint_count = 1;
        return;
    }

    if (this->paint_count == this->paint_capacity) {
        this->paint_capacity += 8;
        this->paint = safe_realloc(this->paint, this->paint_capacity * sizeof(Image *));
    }

    this->paint[this->paint_count] = paint;

    int *index = safe_malloc(sizeof(int));
    index[0] = this->paint_count;

    table_put(this->paint_indices, name, index);

    this->paint_count++;
}

int assets_paint_name_to_index(Assets *this, char *name) {
    int *index = table_get(this->paint_indices, name);
    if (index == NULL) {
        return -1;
    }
    return index[0];
}

Image *assets_paint_get(Assets *this, int index) {
    return this->paint[index];
}

Image *assets_paint_find(Assets *this, char *name) {
    int *index = table_get(this->paint_indices, name);
    if (index == NULL) {
        return NULL;
    }
    return this->paint[index[0]];
}

void assets_delete(Assets *this) {
    free(this->paint);
    table_delete(this->paint_indices);
    free(this);
}
