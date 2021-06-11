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
};

Camera *new_camera(float radius);
void camera_update(Camera *this);

#endif
