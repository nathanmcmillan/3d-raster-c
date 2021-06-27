/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "state.h"

PaintState *new_paint_state(Canvas *canvas, Input *input, Assets *assets) {
    PaintState *this = safe_calloc(1, sizeof(PaintState));
    this->state.canvas = canvas;
    this->state.input = input;
    this->state.assets = assets;
    this->state.update = paint_state_update;
    this->state.draw = paint_state_draw;
    return this;
}

void paint_state_update(void *state) {
    PaintState *this = (PaintState *)state;
    (void *)this;
}

void paint_state_draw(void *state) {
    PaintState *this = (PaintState *)state;
    (void *)this;
}

void paint_state_delete(PaintState *this) {
    free(this);
}
