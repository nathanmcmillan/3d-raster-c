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
#include "sector.h"

typedef struct View View;

struct View {
    float x;
    float y;
    float z;
    float angle;
    float look;
    float radius;
    Thing *target;
    Sector *sector;
};

extern View VIEW;

void ViewInit(float radius);
void ViewThirdPersonUpdate();

#endif
