/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "state.h"

Paint *new_paint(Canvas *canvas, Input *input, Assets *assets) {
    Paint *this = safe_calloc(1, sizeof(Paint));
    this->state.canvas = canvas;
    this->state.input = input;
    this->state.assets = assets;
    this->state.update = paint_update;
    this->state.draw = paint_draw;
    return this;
}

void paint_update(void *state) {
    Paint *this = (Paint *)state;
    (void *)this;
}

void paint_draw(void *state) {
    Paint *this = (Paint *)state;
    (void *)this;
}

void paint_delete(Paint *this) {
    free(this);
}
