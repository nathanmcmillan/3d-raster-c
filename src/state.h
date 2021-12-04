/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef STATE_H
#define STATE_H

#include <math.h>

#include "camera.h"
#include "canvas.h"
#include "hymn.h"
#include "image.h"
#include "input.h"
#include "matrix.h"
#include "mem.h"
#include "resources.h"
#include "sprite.h"
#include "string_util.h"
#include "super.h"
#include "uint_table.h"
#include "vec.h"
#include "wad.h"
#include "world.h"

typedef struct State State;
typedef struct Game Game;
typedef struct Paint Paint;

struct State {
    Hymn *vm;
    Input *input;
    void (*update)(void *);
    void (*draw)(void *);
};

struct Game {
    State state;
    World *world;
    Camera *camera;
    Thing *hero;
};

struct Paint {
    State state;
};

#endif
