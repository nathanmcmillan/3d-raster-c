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

typedef struct State State;

struct State {
    Input in;
    World *w;
    Camera *c;
    Thing *h;
};

State *new_state(World *w);

void state_update(State *this);
void state_render(State *this);

void state_delete(State *this);

#endif
