#include "state.h"

State *new_state(World *w) {

    State *this = safe_calloc(1, sizeof(State));
    this->w = w;
    this->c = new_camera(6);

    int thing_count = w->thing_count;
    Thing **things = w->things;
    for (int i = 0; i < thing_count; i++) {
        if (things[i]->type == THING_TYPE_HERO) {
            this->h = things[i];
            this->c->target = this->h;
            break;
        }
    }

    return this;
}

void state_update(State *this) {
    world_update(this->w);
}

void state_render(State *this) {
}

void state_delete(State *this) {
}
