/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef WAD_PARSER_H
#define WAD_PARSER_H

#include "array.h"
#include "file_io.h"
#include "log.h"
#include "string_util.h"
#include "super.h"
#include "table.h"

typedef struct Wad Wad;

enum WadType {
    WAD_OBJECT,
    WAD_ARRAY,
    WAD_STRING,
};

union WadUnion {
    Table *object;
    Array *array;
    String *string;
};

struct Wad {
    enum WadType type;
    union WadUnion value;
};

Wad *NewWadTable();
Wad *NewWadArray();
Wad *NewWadString(String *value);

bool WadHas(Wad *element, char *key);

Table *WadAsTable(Wad *element);
Array *WadAsArray(Wad *element);
String *WadAsString(Wad *element);
int WadAsInt(Wad *element);
float WadAsFloat(Wad *element);
bool WadAsBool(Wad *element);

void WadAddToTable(Wad *element, char *key, Wad *value);

Wad *WadGetFromTable(Wad *object, char *key);
Wad *WadGetFromArray(Wad *array, unsigned int index);

int WadGetIntFromTable(Wad *object, char *key);
float WadGetFloatFromTable(Wad *object, char *key);
String *WadGetStringFromTable(Wad *object, char *key);
Array *WadGetArrayFromTable(Wad *object, char *key);

int WadSize(Wad *element);

void WadFree(Wad *element);

Wad *WadParse(String *str);

String *WadToString(Wad *element);

#endif
