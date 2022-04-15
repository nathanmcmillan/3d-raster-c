/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "view.h"

View VIEW = {0};

void ViewInit(float radius) {
    VIEW.radius = radius;
}

void ViewThirdPersonUpdate() {
    float sin_x = sinf(VIEW.look);
    float cos_x = cosf(VIEW.look);
    float sin_y = sinf(VIEW.angle);
    float cos_y = cosf(VIEW.angle);

    VIEW.x = VIEW.target->x - VIEW.radius * cos_x * sin_y;
    VIEW.y = VIEW.target->y + VIEW.radius * sin_x + VIEW.target->height;
    VIEW.z = VIEW.target->z + VIEW.radius * cos_x * cos_y;
}
