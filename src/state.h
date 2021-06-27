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

typedef struct State State;
typedef struct GameState GameState;
typedef struct PaintState PaintState;

struct State {
    Canvas *canvas;
    Input *input;
    Assets *assets;
    void (*update)(void *);
    void (*draw)(void *);
};

struct GameState {
    State state;
    World *world;
    Camera *camera;
    Thing *hero;
};

struct PaintState {
    State state;
};

inline void state_update(State *state) {
    state->update(state);
}

inline void state_draw(State *state) {
    state->draw(state);
}

GameState *new_game_state(Canvas *canvas, Input *input, Assets *assets);
void game_state_open(GameState *this, String *content);
void game_state_update(void *state);
void game_state_draw(void *state);
void game_state_delete(GameState *this);

PaintState *new_paint_state(Canvas *canvas, Input *input, Assets *assets);
void paint_state_update(void *state);
void paint_state_draw(void *state);
void paint_state_delete(PaintState *this);

#endif
