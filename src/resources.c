/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "resources.h"

Resources *new_resources() {
    Resources *assets = safe_calloc(1, sizeof(Resources));
    assets->image_table = new_string_table();
    return assets;
}

void resources_add_image(Resources *this, char *name, Image *paint) {
    if (this->image_capacity == 0) {
        this->images = safe_malloc(sizeof(Image *));
        this->images[0] = paint;
        this->image_capacity = 1;
        this->image_count = 1;
    } else {
        if (this->image_count == this->image_capacity) {
            this->image_capacity += 8;
            this->images = safe_realloc(this->images, this->image_capacity * sizeof(Image *));
        }
        this->images[this->image_count] = paint;
        this->image_count++;
    }
    printf("WWW..\n");
    int *index = safe_malloc(sizeof(int));
    index[0] = this->image_count - 1;
    table_put(this->image_table, name, index);
    printf("FFF..\n");
}

int resources_image_name_to_index(Resources *this, char *name) {
    int *index = table_get(this->image_table, name);
    if (index == NULL) {
        return -1;
    }
    return index[0];
}

Image *resources_image_get(Resources *this, int index) {
    return this->images[index];
}

Image *resources_image_find(Resources *this, char *name) {
    int *index = table_get(this->image_table, name);
    if (index == NULL) {
        return NULL;
    }
    return this->images[index[0]];
}

void resources_delete(Resources *this) {
    free(this->images);
    table_delete(this->image_table);
    free(this);
}
