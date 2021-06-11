#ifndef STATE_H
#define STATE_H

#include <math.h>

#include "camera.h"
#include "input.h"
#include "matrix.h"
#include "sprite.h"
#include "uint_table.h"
#include "vector.h"
#include "world.h"
#include "worldbuild.h"

typedef struct GameState GameState;
typedef struct PaintState PaintState;

struct GameState {
    Input *in;
    World *w;
    Camera *c;
    Thing *h;
};

GameState *new_game_state();

void game_state_update(void *this);
void game_state_draw(void *this);

void game_state_delete(GameState *this);

struct PaintState {
    Input *in;
};

PaintState *new_paint_state();

void paint_state_update(void *this);
void paint_state_draw(void *this);

void paint_state_delete(PaintState *this);

#endif
