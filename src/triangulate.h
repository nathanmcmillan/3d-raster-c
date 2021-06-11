/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef TRIANGULATE_H
#define TRIANGULATE_H

#include <float.h>
#include <stdbool.h>
#include <stdlib.h>

#include "array.h"
#include "list.h"
#include "math_util.h"
#include "mem.h"
#include "pie.h"
#include "sector.h"
#include "vec.h"

// #define TRIANGULATE_DEBUG

void triangulate_sector(Sector *s, float scale);

#endif
