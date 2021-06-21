/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "camera.h"

Camera *new_camera(float radius) {
    Camera *c = safe_calloc(1, sizeof(Camera));
    c->radius = radius;
    return c;
}

void camera_update(Camera *this) {

    float sin_x = sinf(this->rx);
    float cos_x = cosf(this->rx);
    float sin_y = sinf(this->ry);
    float cos_y = cosf(this->ry);

    this->x = this->target->x - this->radius * cos_x * sin_y;
    this->y = this->target->y + this->radius * sin_x + this->target->height;
    this->z = this->target->z + this->radius * cos_x * cos_y;
}
