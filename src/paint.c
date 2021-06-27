/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "paint.h"

Paint *new_paint() {
    return safe_calloc(sizeof(Paint), 1);
}

void paint_delete(Paint *this) {
    free(this->pixels);
    free(this);
}
