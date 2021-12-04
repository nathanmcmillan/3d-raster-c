/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "mem.h"

void *Malloc(usize size) {
    void *mem = malloc(size);
    if (mem != NULL) {
        return mem;
    }
    fprintf(stderr, "Malloc failed.\n");
    exit(1);
}

void *Calloc(usize members, usize member_size) {
    void *mem = calloc(members, member_size);
    if (mem != NULL) {
        return mem;
    }
    fprintf(stderr, "Calloc failed.\n");
    exit(1);
}

void *Realloc(void *mem, usize size) {
    mem = realloc(mem, size);
    if (mem != NULL) {
        return mem;
    }
    fprintf(stderr, "Realloc failed.\n");
    exit(1);
}

void Free(void *mem) {
    free(mem);
}
