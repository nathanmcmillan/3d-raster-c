#ifndef WAD_PARSER_H
#define WAD_PARSER_H

#include "array.h"
#include "fileio.h"
#include "pie.h"
#include "string_util.h"
#include "table.h"

enum wad_type { WAD_OBJECT, WAD_ARRAY, WAD_STRING };

typedef enum wad_type wad_type;

typedef table wad_object;
typedef array wad_array;

typedef struct Wad Wad;

union wad_union {
    wad_object *object;
    wad_array *array;
    String *str;
};

struct Wad {
    enum wad_type type;
    union wad_union value;
};

Wad *create_wad_object();
Wad *create_wad_array();
Wad *create_wad_string(String *value);

wad_object *wad_get_object(Wad *element);
wad_array *wad_get_array(Wad *element);
String *wad_get_string(Wad *element);
int wad_get_integer(Wad *element);
float wad_get_float(Wad *element);
bool wad_get_bool(Wad *element);

void wad_add_to_object(Wad *element, char *key, Wad *value);
Wad *wad_get_from_object(Wad *object, char *key);
Wad *wad_get_required_from_object(Wad *object, char *key);
Wad *wad_get_from_array(Wad *array, unsigned int index);
Wad *wad_get_required_from_array(Wad *array, unsigned int index);
table_iterator wad_object_iterator(Wad *object);
unsigned int wad_get_size(Wad *element);

void delete_wad(Wad *element);
Wad *parse_wad(String *str);

String *wad_to_string(Wad *element);

#endif
