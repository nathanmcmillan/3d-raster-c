#ifndef MEM_H
#define MEM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pie.h"

void *safe_malloc(usize size);
void *safe_calloc(usize members, usize member_size);
void *safe_realloc(void *mem, usize size);
void *safe_box(void *stack_struct, usize size);

#endif
