/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "paint.h"

Paint *new_paint(Input *input) {
    Paint *this = Calloc(1, sizeof(Paint));
    this->state.input = input;
    this->state.update = paint_update;
    this->state.draw = paint_draw;
    return this;
}

void paint_update(void *state) {
    Paint *this = (Paint *)state;
    (void)this;
}

void paint_draw(void *state) {
    Paint *this = (Paint *)state;
    (void)this;
}

void paint_delete(Paint *this) {
    Free(this);
}
