#ifndef FILE_IO_H
#define FILE_IO_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mem.h"
#include "pie.h"
#include "string_util.h"

usize file_size(char *path);
usize file_binary_size(char *path);
String *cat(char *path);
void core_write(char *path, char *content);
char *read_binary(char *path, usize *size_pointer);

#endif
