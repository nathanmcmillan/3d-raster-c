/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef STATE_H
#define STATE_H

#include <math.h>

#include "assets.h"
#include "camera.h"
#include "canvas.h"
#include "input.h"
#include "matrix.h"
#include "mem.h"
#include "pie.h"
#include "sprite.h"
#include "string_util.h"
#include "uint_table.h"
#include "vec.h"
#include "wad.h"
#include "world.h"

typedef struct GameState GameState;
typedef struct PaintState PaintState;

struct GameState {
    Canvas *canvas;
    Input *input;
    Assets *assets;
    World *world;
    Camera *camera;
    Thing *hero;
};

GameState *new_game_state(Canvas *canvas, Input *input, Assets *assets);

void game_state_open(GameState *this, String *content);
void game_state_update(void *state);
void game_state_draw(void *state);

void game_state_delete(GameState *this);

struct PaintState {
    Canvas *canvas;
    Input *input;
    Assets *assets;
};

PaintState *new_paint_state(Canvas *canvas, Input *input, Assets *assets);

void paint_state_update(void *state);
void paint_state_draw(void *state);

void paint_state_delete(PaintState *this);

#endif
