/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "camera.h"

Camera *new_camera(float radius) {
    Camera *c = safe_calloc(1, sizeof(Camera));
    c->radius = radius;
    return c;
}

void camera_third_person_update(Camera *camera) {
    float sin_x = sinf(camera->look);
    float cos_x = cosf(camera->look);
    float sin_y = sinf(camera->angle);
    float cos_y = cosf(camera->angle);

    camera->x = camera->target->x - camera->radius * cos_x * sin_y;
    camera->y = camera->target->y + camera->radius * sin_x + camera->target->height;
    camera->z = camera->target->z + camera->radius * cos_x * cos_y;
}
