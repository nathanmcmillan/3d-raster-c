/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "slice.h"

SliceHead *slice_head_init(const usize member_size, const usize length, const usize capacity) {
    usize memory = sizeof(SliceHead) + capacity * member_size;
    SliceHead *head = safe_malloc(memory);
    memset(head, 0, memory);
    head->length = length;
    head->capacity = capacity;
    return head;
}

SliceHead *slice_get_head(const Slice a) {
    return (SliceHead *)a - 1;
}

Slice slice_init(const usize member_size, const usize length, const usize capacity) {
    SliceHead *head = slice_head_init(member_size, length, capacity);
    return (SliceHead *)head + 1;
}

Slice slice_simple_init(const usize member_size, const usize length) {
    return slice_init(member_size, length, length);
}

Slice array_to_slice(void *const array, const usize member_size, const usize length) {
    usize array_memory = length * member_size;
    usize memory = sizeof(SliceHead) + array_memory;
    SliceHead *new_head = safe_malloc(memory);
    new_head->length = length;
    new_head->capacity = length;
    SliceHead *data = (SliceHead *)new_head + 1;
    memcpy(data, array, array_memory);
    return data;
}

void slice_free(const Slice a) {
    SliceHead *head = slice_get_head(a);
    free(head);
}

usize slice_len_size(const Slice a) {
    SliceHead *head = slice_get_head(a);
    return head->length;
}

int slice_len(const Slice a) {
    return (int)slice_len_size(a);
}

usize slice_cap_size(const Slice a) {
    SliceHead *head = slice_get_head(a);
    return head->capacity;
}

int slice_cap(const Slice a) {
    return (int)slice_cap_size(a);
}

SliceHead *slice_resize(const Slice head, const usize member_size, const usize length) {
    usize memory = sizeof(SliceHead) + length * member_size;
    SliceHead *new_head = safe_realloc(head, memory);
    new_head->length = length;
    new_head->capacity = length;
    return new_head;
}

Slice slice_expand(const Slice a, const Slice b) {
    SliceHead *head_a = slice_get_head(a);
    SliceHead *head_b = slice_get_head(b);
    usize length_a = head_a->length;
    usize length_b = head_b->length;
    usize length = length_a + length_b;
    SliceHead *new_head = slice_resize(head_a, sizeof(void *), length);
    SliceHead *data = (SliceHead *)new_head + 1;
    memcpy(data + length_a * sizeof(void *), b, length_b * sizeof(void *));
    return data;
}

Slice slice_push(const Slice a, void *const b) {
    SliceHead *head = slice_get_head(a);
    usize length = head->length + 1;
    if (length > head->capacity) {
        head = slice_resize(head, sizeof(void *), length);
        head->capacity = length;
    }
    head->length = length;
    Slice data = (SliceHead *)head + 1;
    ((SliceHead **)data)[length - 1] = b;
    return data;
}

Slice slice_push_int(const Slice a, const int b) {
    SliceHead *head = slice_get_head(a);
    usize length = head->length + 1;
    if (length > head->capacity) {
        head = slice_resize(head, sizeof(b), length);
        head->capacity = length;
    }
    head->length = length;
    Slice data = (SliceHead *)head + 1;
    ((int *)data)[length - 1] = b;
    return data;
}

Slice slice_push_float(const Slice a, const float b) {
    SliceHead *head = slice_get_head(a);
    usize length = head->length + 1;
    if (length > head->capacity) {
        head = slice_resize(head, sizeof(b), length);
        head->capacity = length;
    }
    head->length = length;
    Slice data = (SliceHead *)head + 1;
    ((float *)data)[length - 1] = b;
    return data;
}

void *slice_pop(const Slice a) {
    SliceHead *head = slice_get_head(a);
    usize length = head->length;
    if (length == 0) {
        return 0;
    }
    head->length--;
    Slice data = (SliceHead *)head + 1;
    return ((SliceHead **)data)[length - 1];
}

int slice_pop_int(const Slice a) {
    SliceHead *head = slice_get_head(a);
    usize length = head->length;
    if (length == 0) {
        return 0;
    }
    head->length--;
    Slice data = (SliceHead *)head + 1;
    return ((int *)data)[length - 1];
}

float slice_pop_float(const Slice a) {
    SliceHead *head = slice_get_head(a);
    usize length = head->length;
    if (length == 0) {
        return 0;
    }
    head->length--;
    Slice data = (SliceHead *)head + 1;
    return ((float *)data)[length - 1];
}
