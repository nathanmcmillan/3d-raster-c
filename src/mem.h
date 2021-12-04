/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef MEM_H
#define MEM_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "super.h"

void *Malloc(usize size);
void *Calloc(usize members, usize member_size);
void *Realloc(void *mem, usize size);
void Free(void *mem);

#endif
