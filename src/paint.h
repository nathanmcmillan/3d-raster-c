/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef PAINT_H
#define PAINT_H

#include "state.h"

Paint *new_paint(Input *input);
void paint_update(void *state);
void paint_draw(void *state);
void paint_delete(Paint *this);

#endif
