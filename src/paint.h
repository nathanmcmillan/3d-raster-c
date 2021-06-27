/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef PAINT_H
#define PAINT_H

#include "mem.h"
#include "pie.h"

typedef struct Paint Paint;

struct Paint {
    i32 width;
    i32 height;
    u8 *pixels;
};

Paint *new_paint();

void paint_delete(Paint *this);

#endif
