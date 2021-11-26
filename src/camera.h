/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef CAMERA_H
#define CAMERA_H

#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "mem.h"
#include "world.h"

typedef struct Camera Camera;

struct Camera {
    float x;
    float y;
    float z;
    float rx;
    float ry;
    float radius;
    Thing *target;
    Sector *sector;
};

Camera *new_camera(float radius);
void camera_update(Camera *this);

#endif
