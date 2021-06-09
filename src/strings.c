/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "strings.h"

static StringHead *string_head_init(usize length, usize capacity) {
    usize memory = sizeof(StringHead) + length + 1;
    StringHead *head = (StringHead *)safe_malloc(memory);
    memset(head, 0, memory);
    head->length = length;
    head->capacity = capacity;
    return head;
}

String *string_init_with_length(char *init, usize length) {
    StringHead *head = string_head_init(length, length);
    char *s = (char *)(head + 1);
    memcpy(s, init, length);
    s[length] = '\0';
    return (String *)s;
}

String *string_allocate(usize length) {
    StringHead *head = string_head_init(length, length);
    char *s = (char *)(head + 1);
    memset(s, '\0', length + 1);
    return (String *)s;
}

String *string_init(char *init) {
    usize len = strlen(init);
    return string_init_with_length(init, len);
}

String *string_copy(String *this) {
    StringHead *head = (StringHead *)((char *)this - sizeof(StringHead));
    return string_init_with_length(this, head->length);
}

usize string_len(String *s) {
    StringHead *head = (StringHead *)((char *)s - sizeof(StringHead));
    return head->length;
}

usize string_cap(String *s) {
    StringHead *head = (StringHead *)((char *)s - sizeof(StringHead));
    return head->capacity;
}

void string_free(String *s) {
    free((char *)s - sizeof(StringHead));
}

String *string_concat(String *a, String *b) {
    usize len1 = string_len(a);
    usize len2 = string_len(b);
    usize len = len1 + len2;
    StringHead *head = string_head_init(len, len);
    char *s = (char *)(head + 1);
    memcpy(s, a, len1);
    memcpy(s + len1, b, len2 + 1);
    s[len] = '\0';
    return (String *)s;
}

String *string_concat_list(String **list, int size) {
    usize len = 0;
    for (int i = 0; i < size; i++) {
        len += string_len(list[i]);
    }
    StringHead *head = string_head_init(len, len);
    char *s = (char *)(head + 1);
    usize pos = 0;
    for (int i = 0; i < size; i++) {
        usize len_i = string_len(list[i]);
        memcpy(s + pos, list[i], len_i);
        pos += len_i;
    }
    s[len] = '\0';
    return (String *)s;
}

String *string_concat_varg(int size, ...) {
    va_list ap;

    usize len = 0;
    va_start(ap, size);
    for (int i = 0; i < size; i++) {
        len += string_len(va_arg(ap, String *));
    }
    va_end(ap);

    StringHead *head = string_head_init(len, len);
    char *s = (char *)(head + 1);

    usize pos = 0;
    va_start(ap, size);
    for (int i = 0; i < size; i++) {
        String *param = va_arg(ap, String *);
        usize len_i = string_len(param);
        memcpy(s + pos, param, len_i);
        pos += len_i;
    }
    va_end(ap);

    s[len] = '\0';
    return (String *)s;
}

String *substring(String *a, usize start, usize end) {
    usize len = end - start;
    StringHead *head = string_head_init(len, len);
    char *s = (char *)(head + 1);
    memcpy(s, a + start, len);
    s[len] = '\0';
    return (String *)s;
}

static StringHead *string_resize(StringHead *head, usize capacity) {
    usize memory = sizeof(StringHead) + capacity + 1;
    StringHead *new = safe_realloc(head, memory);
    new->capacity = capacity;
    return new;
}

String *string_append(String *this, char *b) {
    StringHead *head = (StringHead *)((char *)this - sizeof(StringHead));
    usize len_a = head->length;
    usize len_b = strlen(b);
    usize len = len_a + len_b;
    if (len > head->capacity) {
        head = string_resize(head, len * 2);
    }
    head->length = len;
    char *s = (char *)(head + 1);
    memcpy(s + len_a, b, len_b + 1);
    s[len] = '\0';
    return (String *)s;
}

String *string_append_char(String *this, char b) {
    StringHead *head = (StringHead *)((char *)this - sizeof(StringHead));
    usize len = head->length + 1;
    if (len > head->capacity) {
        head = string_resize(head, len * 2);
    }
    head->length = len;
    char *s = (char *)(head + 1);
    s[len - 1] = b;
    s[len] = '\0';
    return (String *)s;
}

int string_compare(String *a, String *b) {
    usize len_a = string_len(a);
    usize len_b = string_len(b);
    if (len_a == len_b) {
        return strcmp(a, b);
    }
    return (len_a > len_b) ? (int)(len_a - len_b) : -(int)(len_b - len_a);
}

bool string_equal(String *a, String *b) {
    int comparison = string_compare(a, b);
    return comparison == 0;
}

void string_zero(String *this) {
    StringHead *head = (StringHead *)((char *)this - sizeof(StringHead));
    head->length = 0;
    this[0] = '\0';
}

String *char_to_string(char ch) {
    char *str = safe_malloc(2);
    str[0] = ch;
    str[1] = '\0';
    String *s = string_init_with_length(str, 1);
    free(str);
    return s;
}

String *int_to_string(int number) {
    int len = snprintf(NULL, 0, "%d", number);
    char *str = safe_malloc(len + 1);
    snprintf(str, len + 1, "%d", number);
    String *s = string_init_with_length(str, len);
    free(str);
    return s;
}

String *int8_to_string(i8 number) {
    int len = snprintf(NULL, 0, "%" PRId8, number);
    char *str = safe_malloc(len + 1);
    snprintf(str, len + 1, "%" PRId8, number);
    String *s = string_init_with_length(str, len);
    free(str);
    return s;
}

String *int16_to_string(i16 number) {
    int len = snprintf(NULL, 0, "%" PRId16, number);
    char *str = safe_malloc(len + 1);
    snprintf(str, len + 1, "%" PRId16, number);
    String *s = string_init_with_length(str, len);
    free(str);
    return s;
}

String *int32_to_string(i32 number) {
    int len = snprintf(NULL, 0, "%" PRId32, number);
    char *str = safe_malloc(len + 1);
    snprintf(str, len + 1, "%" PRId32, number);
    String *s = string_init_with_length(str, len);
    free(str);
    return s;
}

String *int64_to_string(i64 number) {
    int len = snprintf(NULL, 0, "%" PRId64, number);
    char *str = safe_malloc(len + 1);
    snprintf(str, len + 1, "%" PRId64, number);
    String *s = string_init_with_length(str, len);
    free(str);
    return s;
}

String *size_t_to_string(usize number) {
    int len = snprintf(NULL, 0, "%zu", number);
    char *str = safe_malloc(len + 1);
    snprintf(str, len + 1, "%zu", number);
    String *s = string_init_with_length(str, len);
    free(str);
    return s;
}

String *uint_to_string(unsigned int number) {
    int len = snprintf(NULL, 0, "%u", number);
    char *str = safe_malloc(len + 1);
    snprintf(str, len + 1, "%u", number);
    String *s = string_init_with_length(str, len);
    free(str);
    return s;
}

String *uint8_to_string(u8 number) {
    int len = snprintf(NULL, 0, "%" PRId8, number);
    char *str = safe_malloc(len + 1);
    snprintf(str, len + 1, "%" PRId8, number);
    String *s = string_init_with_length(str, len);
    free(str);
    return s;
}

String *uint16_to_string(u16 number) {
    int len = snprintf(NULL, 0, "%" PRId16, number);
    char *str = safe_malloc(len + 1);
    snprintf(str, len + 1, "%" PRId16, number);
    String *s = string_init_with_length(str, len);
    free(str);
    return s;
}

String *uint32_to_string(u32 number) {
    int len = snprintf(NULL, 0, "%" PRId32, number);
    char *str = safe_malloc(len + 1);
    snprintf(str, len + 1, "%" PRId32, number);
    String *s = string_init_with_length(str, len);
    free(str);
    return s;
}

String *uint64_to_string(u64 number) {
    int len = snprintf(NULL, 0, "%" PRId64, number);
    char *str = safe_malloc(len + 1);
    snprintf(str, len + 1, "%" PRId64, number);
    String *s = string_init_with_length(str, len);
    free(str);
    return s;
}

String *float_to_string(float number) {
    int len = snprintf(NULL, 0, "%f", number);
    char *str = safe_malloc(len + 1);
    snprintf(str, len + 1, "%f", number);
    String *s = string_init_with_length(str, len);
    free(str);
    return s;
}

String *float32_to_string(float number) {
    return float_to_string(number);
}

String *float64_to_string(double number) {
    int len = snprintf(NULL, 0, "%f", number);
    char *str = safe_malloc(len + 1);
    snprintf(str, len + 1, "%f", number);
    String *s = string_init_with_length(str, len);
    free(str);
    return s;
}

bool string_to_bool(String *str) {
    return strcmp(str, "true") == 0;
}

int string_to_int(String *str) {
    return (int)strtol(str, NULL, 10);
}

i8 string_to_int8(String *str) {
    return (i8)strtol(str, NULL, 10);
}

i16 string_to_int16(String *str) {
    return (i16)strtol(str, NULL, 10);
}

i32 string_to_int32(String *str) {
    return (i32)strtol(str, NULL, 10);
}

i64 string_to_int64(String *str) {
    return (i64)strtoll(str, NULL, 10);
}

usize string_to_size_t(String *str) {
    return (usize)strtoll(str, NULL, 10);
}

unsigned int string_to_uint(String *str) {
    return (unsigned int)strtoul(str, NULL, 10);
}

u8 string_to_uint8(String *str) {
    return (u8)strtoul(str, NULL, 10);
}

u16 string_to_uint16(String *str) {
    return (u16)strtoul(str, NULL, 10);
}

u32 string_to_uint32(String *str) {
    return (u32)strtoul(str, NULL, 10);
}

u64 string_to_uint64(String *str) {
    return (u64)strtoull(str, NULL, 10);
}

float string_to_float(String *str) {
    return strtof(str, NULL);
}

float string_to_float32(String *str) {
    return string_to_float(str);
}

double string_to_float64(String *str) {
    return strtod(str, NULL);
}

String *format(String *f, ...) {
    va_list ap;
    va_start(ap, f);
    int len = vsnprintf(NULL, 0, f, ap);
    va_end(ap);
    char *str = safe_malloc((len + 1) * sizeof(char));
    va_start(ap, f);
    len = vsnprintf(str, len + 1, f, ap);
    va_end(ap);
    String *s = string_init_with_length(str, len);
    free(str);
    return s;
}
