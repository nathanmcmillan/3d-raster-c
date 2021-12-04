/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "wad.h"

Wad *new_wad_object() {
    Wad *e = Calloc(1, sizeof(Wad));
    e->type = WAD_OBJECT;
    e->value.object = new_string_table();
    return e;
}

Wad *new_wad_array() {
    Wad *e = Calloc(1, sizeof(Wad));
    e->type = WAD_ARRAY;
    e->value.array = new_array(0);
    return e;
}

Wad *new_wad_string(String *value) {
    Wad *e = Calloc(1, sizeof(Wad));
    e->type = WAD_STRING;
    e->value.str = string_copy(value);
    return e;
}

WadObject *wad_get_object(Wad *element) {
    if (element == NULL) {
        return NULL;
    }
    return element->value.object;
}

bool wad_has(Wad *element, char *key) {
    if (element == NULL) {
        return false;
    }
    return table_has(wad_get_object(element), key);
}

WadArray *wad_get_array(Wad *element) {
    if (element == NULL) {
        return NULL;
    }
    return element->value.array;
}

String *wad_get_string(Wad *element) {
    if (element == NULL) {
        return NULL;
    }
    return element->value.str;
}

int wad_get_int(Wad *element) {
    if (element == NULL) {
        return 0;
    }
    String *str = element->value.str;
    return string_to_int32(str);
}

float wad_get_float(Wad *element) {
    if (element == NULL) {
        return 0.0f;
    }
    String *str = element->value.str;
    return string_to_float(str);
}

bool wad_get_bool(Wad *element) {
    if (element == NULL) {
        return false;
    }
    String *str = element->value.str;
    return string_to_bool(str);
}

void wad_add_to_object(Wad *object, char *key, Wad *value) {
    table_put(wad_get_object(object), new_string(key), value);
}

Wad *wad_get_from_object(Wad *object, char *key) {
    return table_get(wad_get_object(object), key);
}

Wad *wad_get_required_from_object(Wad *object, char *key) {
    Wad *element = wad_get_from_object(object, key);
    if (element == NULL) {
        fprintf(stderr, "Error: Wad key \"%s\" not found: %s\n", key, wad_to_string(object));
        exit(1);
    }
    return element;
}

String *wad_get_string_from_object(Wad *object, char *key) {
    Wad *element = table_get(wad_get_object(object), key);
    return wad_get_string(element);
}

WadArray *wad_get_array_from_object(Wad *object, char *key) {
    Wad *element = table_get(wad_get_object(object), key);
    return wad_get_array(element);
}

Wad *wad_get_from_array(Wad *array, unsigned int index) {
    return array_get(wad_get_array(array), index);
}

Wad *wad_get_required_from_array(Wad *array, unsigned int index) {
    Wad *element = wad_get_from_array(array, index);
    if (element == NULL) {
        fprintf(stderr, "Error: Wad index %d not found: %s\n", index, wad_to_string(array));
        exit(1);
    }
    return element;
}

TableIter wad_object_iterator(Wad *object) {
    return new_table_iterator(wad_get_object(object));
}

usize wad_get_size(Wad *element) {
    switch (element->type) {
    case WAD_OBJECT: return table_size(wad_get_object(element));
    case WAD_ARRAY: return array_size(wad_get_array(element));
    case WAD_STRING: return string_len(wad_get_string(element));
    }
    return 0;
}

void wad_delete(Wad *element) {
    switch (element->type) {
    case WAD_OBJECT: table_delete(wad_get_object(element)); break;
    case WAD_ARRAY: array_delete(wad_get_array(element)); break;
    case WAD_STRING: string_delete(wad_get_string(element)); break;
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
    if (c != '\n' and c != ' ') {
        return i - 1;
    }
    do {
        i++;
        if (i == len) {
            return i;
        }
        c = s[i];
    } while (c == '\n' or c == ' ');
    return i - 1;
}

MaybeWad wad_parse(String *input) {
    printf("parsing wad...\n");
    Wad *wad = new_wad_object();

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
            while (i < len and input[i] != '\n') {
                i++;
            }
        } else if (c == '\n' or c == ' ') {
            if (!parsing_key and pc != '}' and pc != ']') {
                Wad *head = stack->items[0];
                Wad *child = new_wad_string(value);
                if (head->type == WAD_ARRAY) {
                    array_push(wad_get_array(head), child);
                } else {
                    wad_add_to_object(head, key, child);
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
            Wad *map = new_wad_object();
            Wad *head = stack->items[0];
            if (head->type == WAD_ARRAY) {
                array_push(wad_get_array(head), map);
            } else {
                wad_add_to_object(head, key, map);
                string_zero(key);
            }
            array_insert(stack, 0, map);
            parsing_key = true;
            pc = c;
            i = skip_space(input, i);
        } else if (c == '[') {
            Wad *ls = new_wad_array();
            Wad *head = stack->items[0];
            if (head->type == WAD_ARRAY) {
                array_push(wad_get_array(head), ls);
            } else {
                wad_add_to_object(head, key, ls);
                string_zero(key);
            }
            array_insert(stack, 0, ls);
            parsing_key = false;
            pc = c;
            i = skip_space(input, i);
        } else if (c == '}') {
            if (pc != ' ' and pc != ']' and pc != '{' and pc != '}' and pc != '\n') {
                Wad *head = stack->items[0];
                wad_add_to_object(head, key, new_wad_string(value));
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
            if (pc != ' ' and pc != '}' and pc != '[' and pc != ']' and pc != '\n') {
                Wad *head = stack->items[0];
                array_push(wad_get_array(head), new_wad_string(value));
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
                if (e == '"' or e == '\n') {
                    break;
                } else if (e == '\\' and i + 1 < len and input[i + 1] == '"') {
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

    if (pc != ' ' and pc != ']' and pc != '}' and pc != '\n') {
        Wad *head = stack->items[0];
        wad_add_to_object(head, key, new_wad_string(value));
    }

    string_delete(key);
    string_delete(value);

    printf("done parsing wad...\n");
    return (MaybeWad){wad, NULL};
}

String *wad_to_string(Wad *element) {
    switch (element->type) {
    case WAD_OBJECT: {
        WadObject *map = wad_get_object(element);
        String *str = new_string("{");
        TableIter iter = new_table_iterator(map);
        while (table_iterator_has_next(&iter)) {
            TablePair pair = table_iterator_next(&iter);
            String *in = wad_to_string(pair.value);
            str = string_append(str, pair.key);
            if (in[0] != '[' and in[0] != '{') {
                str = string_append_char(str, ':');
            }
            str = string_append(str, in);
            if (table_iterator_has_next(&iter)) {
                str = string_append_char(str, ' ');
            }
            string_delete(in);
        }
        str = string_append_char(str, '}');
        return str;
    }
    case WAD_ARRAY: {
        WadArray *ls = wad_get_array(element);
        String *str = new_string("[");
        usize len = ls->length;
        for (usize i = 0; i < len; i++) {
            String *in = wad_to_string(ls->items[i]);
            str = string_append(str, in);
            if (i < len - 1) {
                str = string_append_char(str, ' ');
            }
            string_delete(in);
        }
        str = string_append_char(str, ']');
        return str;
    }
    case WAD_STRING: return string_copy(wad_get_string(element));
    }
    return NULL;
}
