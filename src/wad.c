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
    e->value.array = NewArray(0);
    return e;
}

Wad *NewWadString(String *value) {
    Wad *e = Calloc(1, sizeof(Wad));
    e->type = WAD_STRING;
    e->value.string = StringCopy(value);
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

int WadAsInt(Wad *element) {
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
    TablePut(WadAsTable(object), NewString(key), value);
}

Wad *WadGetFromTable(Wad *object, char *key) {
    return TableGet(WadAsTable(object), key);
}

Wad *WadGetFromArray(Wad *array, unsigned int index) {
    return ArrayGet(WadAsArray(array), index);
}

int WadGetIntFromTable(Wad *object, char *key) {
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

int WadSize(Wad *element) {
    switch (element->type) {
    case WAD_OBJECT: return table_size(WadAsTable(element));
    case WAD_ARRAY: return ArraySize(WadAsArray(element));
    case WAD_STRING: return StringLen(WadAsString(element));
    }
    return 0;
}

void WadFree(Wad *element) {
    switch (element->type) {
    case WAD_OBJECT: table_delete(WadAsTable(element)); break;
    case WAD_ARRAY: ArrayFree(WadAsArray(element)); break;
    case WAD_STRING: StringFree(WadAsString(element)); break;
    }
    Free(element);
}

static int skip_space(String *s, int i) {
    int len = StringLen(s);
    if (i + 1 >= len) {
        fprintf(stderr, "Wad error at index %d, for: %s", i, s);
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

Wad *WadParse(String *input) {
    printf("parsing wad...\n");
    Wad *wad = NewWadTable();

    Array *stack = NewArray(0);
    ArrayPush(stack, wad);

    String *key = NewString("");
    String *value = NewString("");

    char pc = '\0';
    bool parsing_key = true;

    int len = StringLen(input);

    for (int i = 0; i < len; i++) {
        char c = input[i];
        if (c == '\n' || c == ' ') {
            if (!parsing_key && pc != '}' && pc != ']') {
                if (ArrayIsEmpty(stack)) {
                    ERROR("Wad: Empty stack");
                    exit(1);
                }
                Wad *head = stack->items[0];
                Wad *child = NewWadString(value);
                if (head->type == WAD_ARRAY) {
                    ArrayPush(WadAsArray(head), child);
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
            if (ArrayIsEmpty(stack)) {
                ERROR("Wad: Empty stack");
                exit(1);
            }
            Wad *map = NewWadTable();
            Wad *head = stack->items[0];
            if (head->type == WAD_ARRAY) {
                ArrayPush(WadAsArray(head), map);
            } else {
                WadAddToTable(head, key, map);
                string_zero(key);
            }
            ArrayInsert(stack, 0, map);
            parsing_key = true;
            pc = c;
            i = skip_space(input, i);
        } else if (c == '[') {
            if (ArrayIsEmpty(stack)) {
                ERROR("Wad: Empty stack");
                exit(1);
            }
            Wad *ls = NewWadArray();
            Wad *head = stack->items[0];
            if (head->type == WAD_ARRAY) {
                ArrayPush(WadAsArray(head), ls);
            } else {
                WadAddToTable(head, key, ls);
                string_zero(key);
            }
            ArrayInsert(stack, 0, ls);
            parsing_key = false;
            pc = c;
            i = skip_space(input, i);
        } else if (c == '}') {
            if (pc != ' ' && pc != ']' && pc != '{' && pc != '}' && pc != '\n') {
                if (ArrayIsEmpty(stack)) {
                    ERROR("Wad: Empty stack");
                    exit(1);
                }
                Wad *head = stack->items[0];
                WadAddToTable(head, key, NewWadString(value));
                string_zero(key);
                string_zero(value);
            }
            ArrayRemoveAt(stack, 0);
            if (ArrayIsEmpty(stack)) {
                ERROR("Wad: Empty stack");
                exit(1);
            }
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
                if (ArrayIsEmpty(stack)) {
                    ERROR("Wad: Empty stack");
                    exit(1);
                }
                Wad *head = stack->items[0];
                ArrayPush(WadAsArray(head), NewWadString(value));
                string_zero(value);
            }
            ArrayRemoveAt(stack, 0);
            if (ArrayIsEmpty(stack)) {
                ERROR("Wad: Empty stack");
                exit(1);
            }
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
                    value = StringAppendChar(value, '"');
                    i += 2;
                    e = input[i];
                } else {
                    value = StringAppendChar(value, e);
                    i++;
                    e = input[i];
                }
            }
            pc = c;
        } else if (parsing_key) {
            pc = c;
            key = StringAppendChar(key, c);
        } else {
            pc = c;
            value = StringAppendChar(value, c);
        }
    }

    if (ArrayIsEmpty(stack)) {
        ERROR("Wad: Empty stack");
        exit(1);
    }

    if (pc != ' ' && pc != ']' && pc != '}' && pc != '\n') {
        Wad *head = stack->items[0];
        WadAddToTable(head, key, NewWadString(value));
    }

    StringFree(key);
    StringFree(value);

    printf("done parsing wad...\n");
    return wad;
}

String *WadToString(Wad *element) {
    switch (element->type) {
    case WAD_OBJECT: {
        Table *map = WadAsTable(element);
        String *string = NewString("{");
        TableIter iter = NewTableIterator(map);
        while (TableIteratorHasNext(&iter)) {
            TablePair pair = TableIteratorNext(&iter);
            String *in = WadToString(pair.value);
            string = StringAppend(string, pair.key);
            if (in[0] != '[' && in[0] != '{') {
                string = StringAppendChar(string, ':');
            }
            string = StringAppend(string, in);
            if (TableIteratorHasNext(&iter)) {
                string = StringAppendChar(string, ' ');
            }
            StringFree(in);
        }
        string = StringAppendChar(string, '}');
        return string;
    }
    case WAD_ARRAY: {
        Array *ls = WadAsArray(element);
        String *string = NewString("[");
        int size = ls->size;
        for (int i = 0; i < size; i++) {
            String *in = WadToString(ls->items[i]);
            string = StringAppend(string, in);
            if (i < size - 1) {
                string = StringAppendChar(string, ' ');
            }
            StringFree(in);
        }
        string = StringAppendChar(string, ']');
        return string;
    }
    case WAD_STRING: return StringCopy(WadAsString(element));
    }
    return NULL;
}
