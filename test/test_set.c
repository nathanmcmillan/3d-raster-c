#include "test_set.h"

typedef struct {
    int value;
} Integer;

static bool integer_equal(void *a, void *b) {
    return ((Integer *)a)->value == ((Integer *)b)->value;
}

static unsigned long integer_hashcode(void *key) {
    return ((Integer *)key)->value;
}

static char *test_iterator() {
    char *x = "foo";
    char *y = "bar";
    char *z = "zoo";

    Set *tab = new_set(&set_string_equal, &set_string_hashcode);

    set_add(tab, x);
    set_add(tab, y);
    set_add(tab, z);

    SetIterator iter = new_set_iterator(tab);

    ASSERT("(1) has next == true", set_iterator_has_next(&iter));
    void *key = set_iterator_next(&iter);
    ASSERT("(1) key not null", key != NULL);
    ASSERT("(1) key equals ?", strcmp(key, "foo") || strcmp(key, "bar") || strcmp(key, "zoo"));

    ASSERT("(2) has next == true", set_iterator_has_next(&iter));
    key = set_iterator_next(&iter);
    ASSERT("(2) key not null", key != NULL);
    ASSERT("(2) key equals ?", strcmp(key, "foo") || strcmp(key, "bar") || strcmp(key, "zoo"));

    key = set_iterator_next(&iter);
    ASSERT("(3) key not null", key != NULL);
    ASSERT("(3) key equals ?", strcmp(key, "foo") || strcmp(key, "bar") || strcmp(key, "zoo"));

    ASSERT("(3) has next == false", set_iterator_has_next(&iter) == false);

    return 0;
}

static char *test_stress() {

    unsigned int size = 10000;

    Integer *keys = safe_calloc(size, sizeof(Integer));

    Set *tab = new_set(&integer_equal, &integer_hashcode);

    for (unsigned int i = 0; i < size; i++) {
        keys[i] = (Integer){i};

        set_add(tab, &keys[i]);
    }

    ASSERT("size", set_size(tab) == size);

    for (unsigned int i = 0; i < size; i++) {
        Integer key = (Integer){i};
        ASSERT("has(i)", set_has(tab, &key) == true);
    }

    for (unsigned int i = 0; i < size; i++) {
        Integer key = (Integer){size + i};
        ASSERT("has(size + i)", set_has(tab, &key) == false);
    }

    return 0;
}

static char *test_string() {
    char *x = "foo";
    char *y = "bar";
    char *z = "zoo";

    Set *tab = new_set(&set_string_equal, &set_string_hashcode);

    set_add(tab, x);
    set_add(tab, y);
    set_add(tab, z);

    ASSERT("size == 3", set_size(tab) == 3);

    ASSERT("has(z)", set_has(tab, z) == true);
    ASSERT("has(x)", set_has(tab, x) == true);
    ASSERT("has(y)", set_has(tab, y) == true);
    ASSERT("has(baz)", set_has(tab, "baz") == false);

    set_remove(tab, x);
    set_remove(tab, y);
    set_remove(tab, z);

    ASSERT("size == 0", set_size(tab) == 0);

    ASSERT("has(z) == false", set_has(tab, z) == false);
    ASSERT("has(x) == false", set_has(tab, x) == false);
    ASSERT("has(y) == false", set_has(tab, y) == false);
    ASSERT("has(bas)", set_has(tab, "baz") == false);

    return 0;
}

static char *test_address() {
    Integer x = {4};
    Integer y = {6};
    Integer z = {12};

    Set *tab = new_set(&set_address_equal, &set_address_hashcode);

    set_add(tab, &x);
    set_add(tab, &y);
    set_add(tab, &z);

    ASSERT("size == 3", set_size(tab) == 3);

    ASSERT("has(z)", set_has(tab, &z) == true);
    ASSERT("has(x)", set_has(tab, &x) == true);
    ASSERT("has(y)", set_has(tab, &y) == true);
    ASSERT("has(w)", set_has(tab, &(Integer){99}) == false);

    return 0;
}

char *test_set_all() {
    TEST(test_address);
    TEST(test_string);
    TEST(test_stress);
    TEST(test_iterator);
    return 0;
}
