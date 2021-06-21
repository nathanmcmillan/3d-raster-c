/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef WAD_PARSER_H
#define WAD_PARSER_H

#include "array.h"
#include "fileio.h"
#include "pie.h"
#include "string_util.h"
#include "table.h"

typedef Table WadObject;
typedef Array WadArray;

typedef struct Wad Wad;
typedef struct MaybeWad MaybeWad;

enum WadType { WAD_OBJECT, WAD_ARRAY, WAD_STRING };

union WadUnion {
    WadObject *object;
    WadArray *array;
    String *str;
};

struct Wad {
    enum WadType type;
    union WadUnion value;
};

struct MaybeWad {
    Wad *wad;
    char *error;
};

Wad *new_wad_object();
Wad *new_wad_array();
Wad *new_wad_string(String *value);

WadObject *wad_get_object(Wad *element);
WadArray *wad_get_array(Wad *element);
String *wad_get_string(Wad *element);
int wad_get_int(Wad *element);
float wad_get_float(Wad *element);
bool wad_get_bool(Wad *element);

void wad_add_to_object(Wad *element, char *key, Wad *value);
Wad *wad_get_from_object(Wad *object, char *key);
Wad *wad_get_required_from_object(Wad *object, char *key);
String *wad_get_string_from_object(Wad *object, char *key);
WadArray *wad_get_array_from_object(Wad *object, char *key);
Wad *wad_get_from_array(Wad *array, unsigned int index);
Wad *wad_get_required_from_array(Wad *array, unsigned int index);
TableIter wad_object_iterator(Wad *object);
unsigned int wad_get_size(Wad *element);

void wad_delete(Wad *element);

MaybeWad wad_parse(String *str);

String *wad_to_string(Wad *element);

#endif
