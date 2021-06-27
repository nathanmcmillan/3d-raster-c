#include "test_array.h"

typedef struct {
    int value;
} Integer;

static bool int_find(void *x, void *y) {
    return ((Integer *)x)->value == ((Integer *)y)->value;
}

static char *test_capacity() {
    Integer x = {4};
    Integer y = {8};
    Integer z = {6};

    Array *ls = create_array_with_capacity(2, 3);

    ASSERT("length == 2", ls->length == 2);
    ASSERT("capacity == 3", ls->capacity == 3);

    array_push(ls, &x);
    array_push(ls, &y);
    array_push(ls, &z);

    ASSERT("length == 5", ls->length == 5);
    ASSERT("capacity >= 5", ls->capacity >= 5);

    delete_array(ls);

    return 0;
}

static char *test_copy() {
    Integer x = {4};
    Integer y = {8};
    Integer z = {6};

    Array *ls = create_array(0);
    array_push(ls, &x);
    array_push(ls, &y);
    array_push(ls, &z);

    Integer **integers = (Integer **)array_copy_items(ls);

    delete_array(ls);

    ASSERT("integers[0] == 4", ((Integer *)integers[0])->value == 4);
    ASSERT("integers[1] == 8", ((Integer *)integers[1])->value == 8);
    ASSERT("integers[2] == 6", ((Integer *)integers[2])->value == 6);

    return 0;
}

static char *test_is_empty_clear() {

    Array *ls = create_array(0);
    ASSERT("is empty", array_is_empty(ls));

    Integer x = {4};
    array_push(ls, &x);
    ASSERT("is not empty", array_not_empty(ls));

    array_clear(ls);
    ASSERT("is empty", array_is_empty(ls));

    delete_array(ls);

    return 0;
}

static char *test_find() {
    Integer x = {4};
    Integer y = {8};
    Integer z = {6};
    Integer w = {0};

    Array *ls = create_array(0);
    array_push(ls, &x);
    array_push(ls, &y);
    array_push(ls, &z);
    array_push(ls, &w);

    Integer n = {9};

    ASSERT("find(9) == NULL", array_find(ls, int_find, &n) == NULL);

    delete_array(ls);

    return 0;
}

static int int_sort(void *x, void *y) {
    return ((Integer *)x)->value < ((Integer *)y)->value ? -1 : 1;
}

static char *test_sort() {
    Integer x = {4};
    Integer y = {8};
    Integer z = {6};
    Integer w = {0};

    Array *ls = create_array(0);
    array_insert_sort(ls, int_sort, &x);
    array_insert_sort(ls, int_sort, &y);
    array_insert_sort(ls, int_sort, &z);
    array_insert_sort(ls, int_sort, &w);

    ASSERT("size == 4", array_size(ls) == 4);
    ASSERT("capacity >= length", ls->capacity >= ls->length);

    ASSERT("get(0) == 0", ((Integer *)array_get(ls, 0))->value == 0);
    ASSERT("get(1) == 4", ((Integer *)array_get(ls, 1))->value == 4);
    ASSERT("get(2) == 6", ((Integer *)array_get(ls, 2))->value == 6);
    ASSERT("get(3) == 8", ((Integer *)array_get(ls, 3))->value == 8);

    delete_array(ls);

    return 0;
}

static char *test_insert_remove() {
    Integer x = {4};
    Integer y = {6};
    Integer z = {12};

    Array *ls = create_array(0);
    array_insert(ls, 0, &x);
    array_insert(ls, 0, &y);
    array_insert(ls, 0, &z);

    ASSERT("size == 3", array_size(ls) == 3);
    ASSERT("capacity >= length", ls->capacity >= ls->length);

    ASSERT("get(0) == 12", ((Integer *)array_get(ls, 0))->value == 12);
    ASSERT("get(1) == 6", ((Integer *)array_get(ls, 1))->value == 6);
    ASSERT("get(2) == 4", ((Integer *)array_get(ls, 2))->value == 4);

    array_remove_index(ls, 0);
    ASSERT("remove(0) + get(0) == 6", ((Integer *)array_get(ls, 0))->value == 6);
    array_remove_index(ls, 0);
    ASSERT("remove(0) + get(0) == 4", ((Integer *)array_get(ls, 0))->value == 4);
    array_remove_index(ls, 0);

    ASSERT("size == 0", array_size(ls) == 0);
    ASSERT("capacity >= length", ls->capacity >= ls->length);

    delete_array(ls);

    return 0;
}

static char *test_push_pop() {
    Integer x = {4};
    Integer y = {6};
    Integer z = {12};

    Array *ls = create_array(0);
    array_push(ls, &x);
    array_push(ls, &y);
    array_push(ls, &z);

    ASSERT("size == 3", array_size(ls) == 3);
    ASSERT("capacity >= length", ls->capacity >= ls->length);

    ASSERT("get(0) == 4", ((Integer *)array_get(ls, 0))->value == 4);
    ASSERT("get(1) == 6", ((Integer *)array_get(ls, 1))->value == 6);
    ASSERT("get(2) == 12", ((Integer *)array_get(ls, 2))->value == 12);

    ASSERT("1st pop", ((Integer *)array_pop(ls))->value == 12);
    ASSERT("2nd pop", ((Integer *)array_pop(ls))->value == 6);
    ASSERT("3rdt pop", ((Integer *)array_pop(ls))->value == 4);

    ASSERT("size == 0", array_size(ls) == 0);
    ASSERT("capacity >= length", ls->capacity >= ls->length);

    delete_array(ls);

    return 0;
}

char *test_array_all() {
    TEST(test_push_pop);
    TEST(test_insert_remove);
    TEST(test_find);
    TEST(test_sort);
    TEST(test_is_empty_clear);
    TEST(test_copy);
    TEST(test_capacity);
    return 0;
}
