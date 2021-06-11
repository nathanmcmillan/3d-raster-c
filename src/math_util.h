/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef MATH_UTIL_H
#define MATH_UTIL_H

#include <math.h>
#include <stdlib.h>

#define MATH_PI 3.14159265358979323846
#define MATH_TAU (MATH_PI * 2.0)

#define FLOAT_MATH_PI 3.14159265358979323846f
#define FLOAT_MATH_HALF_PI (FLOAT_MATH_PI * 0.5f)
#define FLOAT_MATH_QUARTER_PI (FLOAT_MATH_PI * 0.25f)
#define FLOAT_MATH_TAU (FLOAT_MATH_PI * 2.0f)

#define FLOAT_PRECISION 0.00000001f

#define FLOAT_ZERO(F) fabs(F) < FLOAT_PRECISION
#define FLOAT_NOT_ZERO(F) fabs(F) > FLOAT_PRECISION
#define FLOAT_EQUAL(X, Y) fabs(X - Y) < FLOAT_PRECISION

#define DEGREE_TO_RADIAN(D) (D * (MATH_PI / 180.0))
#define RADIAN_TO_DEGREE(R) (R * (180.0 / MATH_PI))

float rand_float();
float lerp(float x, float y, float t);

#endif
