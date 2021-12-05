/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef GAME_H
#define GAME_H

#include <math.h>

#include "camera.h"
#include "canvas.h"
#include "draw.h"
#include "hymn.h"
#include "image.h"
#include "input.h"
#include "matrix.h"
#include "mem.h"
#include "resources.h"
#include "string_util.h"
#include "super.h"
#include "vec.h"
#include "wad.h"
#include "world.h"

enum Mode {
    MODE_GAME,
    MODE_PAINT,
};

extern Hymn *VM;

extern Input INPUT;

extern World *WORLD;
extern Camera *CAMERA;
extern Thing *PLAYER;

extern enum Mode MODE;

void GameInit();
void GameOpen(String *content);
void GameTick();
void GameFree();

void PaintInit();
void PaintTick();
void PaintFree();

#endif
