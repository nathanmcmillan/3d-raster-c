/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "math_util.h"

float rand_float() {
    return (float)rand() / (float)RAND_MAX;
}

float lerp(float x, float y, float t) {
    return x + t * (y - x);
}
