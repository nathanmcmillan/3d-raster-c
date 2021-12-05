/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "wad.h"

Wad *NewWadTable() {
    Wad *e = Calloc(1, sizeof(Wad));
    e->type = WAD_OBJECT;
    e->value.object = new_string_table();
    return e;
}

Wad *NewWadArray() {
    Wad *e = Calloc(1, sizeof(Wad));
    e->type = WAD_ARRAY;
    e->value.array = new_array(0);
    return e;
}

Wad *NewWadString(String *value) {
    Wad *e = Calloc(1, sizeof(Wad));
    e->type = WAD_STRING;
    e->value.string = string_copy(value);
    return e;
}

Table *WadAsTable(Wad *element) {
    if (element == NULL) {
        return NULL;
    }
    return element->value.object;
}

bool WadHas(Wad *element, char *key) {
    if (element == NULL) {
        return false;
    }
    return TableHas(element->value.object, key);
}

Array *WadAsArray(Wad *element) {
    if (element == NULL) {
        return NULL;
    }
    return element->value.array;
}

String *WadAsString(Wad *element) {
    if (element == NULL) {
        return NULL;
    }
    return element->value.string;
}

i32 WadAsInt(Wad *element) {
    if (element == NULL) {
        return 0;
    }
    String *string = element->value.string;
    return string_to_int32(string);
}

float WadAsFloat(Wad *element) {
    if (element == NULL) {
        return 0.0f;
    }
    String *string = element->value.string;
    return string_to_float(string);
}

bool WadAsBool(Wad *element) {
    if (element == NULL) {
        return false;
    }
    String *string = element->value.string;
    return string_to_bool(string);
}

void WadAddToTable(Wad *object, char *key, Wad *value) {
    TablePut(WadAsTable(object), new_string(key), value);
}

Wad *WadGetFromTable(Wad *object, char *key) {
    return TableGet(WadAsTable(object), key);
}

Wad *WadGetFromArray(Wad *array, unsigned int index) {
    return ArrayGet(WadAsArray(array), index);
}

i32 WadGetIntFromTable(Wad *object, char *key) {
    return WadAsInt(WadGetFromTable(object, key));
}

float WadGetFloatFromTable(Wad *object, char *key) {
    return WadAsFloat(WadGetFromTable(object, key));
}

String *WadGetStringFromTable(Wad *object, char *key) {
    return WadAsString(WadGetFromTable(object, key));
}

Array *WadGetArrayFromTable(Wad *object, char *key) {
    return WadAsArray(WadGetFromTable(object, key));
}

usize WadSize(Wad *element) {
    switch (element->type) {
    case WAD_OBJECT: return table_size(WadAsTable(element));
    case WAD_ARRAY: return array_size(WadAsArray(element));
    case WAD_STRING: return string_len(WadAsString(element));
    }
    return 0;
}

void WadFree(Wad *element) {
    switch (element->type) {
    case WAD_OBJECT: table_delete(WadAsTable(element)); break;
    case WAD_ARRAY: array_delete(WadAsArray(element)); break;
    case WAD_STRING: StringFree(WadAsString(element)); break;
    }
    Free(element);
}

static usize skip_space(String *s, usize i) {
    usize len = string_len(s);
    if (i + 1 >= len) {
        fprintf(stderr, "Wad error at index %zu, for: %s", i, s);
        exit(1);
    }
    i++;
    char c = s[i];
    if (c != '\n' && c != ' ') {
        return i - 1;
    }
    do {
        i++;
        if (i == len) {
            return i;
        }
        c = s[i];
    } while (c == '\n' || c == ' ');
    return i - 1;
}

MaybeWad WadParse(String *input) {
    printf("parsing wad...\n");
    Wad *wad = NewWadTable();

    Array *stack = new_array(0);
    array_push(stack, wad);

    String *key = new_string("");
    String *value = new_string("");

    char pc = '\0';
    bool parsing_key = true;

    usize len = string_len(input);

    for (usize i = 0; i < len; i++) {
        char c = input[i];
        if (c == '#') {
            pc = c;
            i++;
            while (i < len && input[i] != '\n') {
                i++;
            }
        } else if (c == '\n' || c == ' ') {
            if (!parsing_key && pc != '}' && pc != ']') {
                Wad *head = stack->items[0];
                Wad *child = NewWadString(value);
                if (head->type == WAD_ARRAY) {
                    array_push(WadAsArray(head), child);
                } else {
                    WadAddToTable(head, key, child);
                    string_zero(key);
                    parsing_key = true;
                }
                string_zero(value);
            }
            pc = c;
            i = skip_space(input, i);
        } else if (c == ':') {
            parsing_key = false;
            pc = c;
            i = skip_space(input, i);
        } else if (c == '{') {
            Wad *map = NewWadTable();
            Wad *head = stack->items[0];
            if (head->type == WAD_ARRAY) {
                array_push(WadAsArray(head), map);
            } else {
                WadAddToTable(head, key, map);
                string_zero(key);
            }
            array_insert(stack, 0, map);
            parsing_key = true;
            pc = c;
            i = skip_space(input, i);
        } else if (c == '[') {
            Wad *ls = NewWadArray();
            Wad *head = stack->items[0];
            if (head->type == WAD_ARRAY) {
                array_push(WadAsArray(head), ls);
            } else {
                WadAddToTable(head, key, ls);
                string_zero(key);
            }
            array_insert(stack, 0, ls);
            parsing_key = false;
            pc = c;
            i = skip_space(input, i);
        } else if (c == '}') {
            if (pc != ' ' && pc != ']' && pc != '{' && pc != '}' && pc != '\n') {
                Wad *head = stack->items[0];
                WadAddToTable(head, key, NewWadString(value));
                string_zero(key);
                string_zero(value);
            }
            array_remove_index(stack, 0);
            Wad *head = stack->items[0];
            if (head->type == WAD_ARRAY) {
                parsing_key = false;
            } else {
                parsing_key = true;
            }
            pc = c;
            i = skip_space(input, i);
        } else if (c == ']') {
            if (pc != ' ' && pc != '}' && pc != '[' && pc != ']' && pc != '\n') {
                Wad *head = stack->items[0];
                array_push(WadAsArray(head), NewWadString(value));
                string_zero(value);
            }
            array_remove_index(stack, 0);
            Wad *head = stack->items[0];
            if (head->type == WAD_ARRAY) {
                parsing_key = false;
            } else {
                parsing_key = true;
            }
            pc = c;
            i = skip_space(input, i);
        } else if (c == '"') {
            i++;
            if (i == len) {
                break;
            }
            char e = input[i];
            while (i < len) {
                if (e == '"' || e == '\n') {
                    break;
                } else if (e == '\\' && i + 1 < len && input[i + 1] == '"') {
                    value = string_append_char(value, '"');
                    i += 2;
                    e = input[i];
                } else {
                    value = string_append_char(value, e);
                    i++;
                    e = input[i];
                }
            }
            pc = c;
        } else if (parsing_key) {
            pc = c;
            key = string_append_char(key, c);
        } else {
            pc = c;
            value = string_append_char(value, c);
        }
    }

    if (pc != ' ' && pc != ']' && pc != '}' && pc != '\n') {
        Wad *head = stack->items[0];
        WadAddToTable(head, key, NewWadString(value));
    }

    StringFree(key);
    StringFree(value);

    printf("done parsing wad...\n");
    return (MaybeWad){wad, NULL};
}

String *WadToString(Wad *element) {
    switch (element->type) {
    case WAD_OBJECT: {
        Table *map = WadAsTable(element);
        String *string = new_string("{");
        TableIter iter = new_table_iterator(map);
        while (table_iterator_has_next(&iter)) {
            TablePair pair = table_iterator_next(&iter);
            String *in = WadToString(pair.value);
            string = string_append(string, pair.key);
            if (in[0] != '[' && in[0] != '{') {
                string = string_append_char(string, ':');
            }
            string = string_append(string, in);
            if (table_iterator_has_next(&iter)) {
                string = string_append_char(string, ' ');
            }
            StringFree(in);
        }
        string = string_append_char(string, '}');
        return string;
    }
    case WAD_ARRAY: {
        Array *ls = WadAsArray(element);
        String *string = new_string("[");
        usize len = ls->length;
        for (usize i = 0; i < len; i++) {
            String *in = WadToString(ls->items[i]);
            string = string_append(string, in);
            if (i < len - 1) {
                string = string_append_char(string, ' ');
            }
            StringFree(in);
        }
        string = string_append_char(string, ']');
        return string;
    }
    case WAD_STRING: return string_copy(WadAsString(element));
    }
    return NULL;
}
