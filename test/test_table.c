#include "test_table.h"

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

    Integer w = {14};
    Integer k = {16};
    Integer n = {18};

    Table *tab = new_table(&table_string_equal, &table_string_hashcode);

    table_put(tab, x, &w);
    table_put(tab, y, &k);
    table_put(tab, z, &n);

    TableIter iter = new_table_iterator(tab);

    ASSERT("(1) has next == true", table_iterator_has_next(&iter));
    TablePair pair = table_iterator_next(&iter);
    ASSERT("(1) pair not null", pair.value != NULL);
    int value = ((Integer *)pair.value)->value;
    ASSERT("(1) pair equals 14 or 16 or 18", value == 14 || value == 16 || value == 18);

    ASSERT("(2) has next == true", table_iterator_has_next(&iter));
    pair = table_iterator_next(&iter);
    ASSERT("(2) pair not null", pair.value != NULL);
    value = ((Integer *)pair.value)->value;
    ASSERT("(2) pair equals 14 or 16 or 18", value == 14 || value == 16 || value == 18);

    pair = table_iterator_next(&iter);
    ASSERT("(3) pair not null", pair.value != NULL);
    value = ((Integer *)pair.value)->value;
    ASSERT("(3) pair equals 14 or 16 or 18", value == 14 || value == 16 || value == 18);

    ASSERT("(3) has next == false", table_iterator_has_next(&iter) == false);

    return 0;
}

static char *test_remove() {
    char *x = "foo";
    char *y = "bar";
    char *z = "zoo";

    Integer w = {14};
    Integer k = {16};
    Integer n = {18};

    Table *tab = new_table(&table_string_equal, &table_string_hashcode);

    table_put(tab, x, &w);
    table_put(tab, y, &k);
    table_put(tab, z, &n);

    table_remove(tab, x);
    table_remove(tab, y);
    table_remove(tab, z);

    ASSERT("size == 0", table_size(tab) == 0);

    ASSERT("has(z) == false", table_has(tab, z) == false);
    ASSERT("has(x) == false", table_has(tab, x) == false);
    ASSERT("has(y) == false", table_has(tab, y) == false);

    return 0;
}

static char *test_stress() {

    unsigned int size = 10000;

    Integer *keys = safe_calloc(size, sizeof(Integer));
    Integer *values = safe_calloc(size, sizeof(Integer));

    Table *tab = new_table(&integer_equal, &integer_hashcode);

    for (unsigned int i = 0; i < size; i++) {
        keys[i] = (Integer){i};
        values[i] = (Integer){rand()};

        table_put(tab, &keys[i], &values[i]);
    }

    ASSERT("size", table_size(tab) == size);

    for (unsigned int i = 0; i < size; i++) {
        Integer key = (Integer){keys[i].value};
        ASSERT("get(i) == value[i]", table_get(tab, &key) == &values[i]);
    }

    return 0;
}

static char *test_string() {
    char *x = "foo";
    char *y = "bar";
    char *z = "zoo";

    Integer w = {14};
    Integer k = {16};
    Integer n = {18};

    Table *tab = new_table(&table_string_equal, &table_string_hashcode);

    table_put(tab, x, &w);
    table_put(tab, y, &k);
    table_put(tab, z, &n);

    ASSERT("size == 3", table_size(tab) == 3);

    ASSERT("get(z) == n", table_get(tab, z) == &n);
    ASSERT("get(x) == w", table_get(tab, x) == &w);
    ASSERT("get(y) == k", table_get(tab, y) == &k);

    return 0;
}

static char *test_address() {
    Integer x = {4};
    Integer y = {6};
    Integer z = {12};

    Integer w = {14};
    Integer k = {16};
    Integer n = {18};

    Table *tab = new_table(&table_address_equal, &table_address_hashcode);

    table_put(tab, &x, &w);
    table_put(tab, &y, &k);
    table_put(tab, &z, &n);

    ASSERT("size == 3", table_size(tab) == 3);

    ASSERT("get(z) == n", table_get(tab, &z) == &n);
    ASSERT("get(x) == w", table_get(tab, &x) == &w);
    ASSERT("get(y) == k", table_get(tab, &y) == &k);

    return 0;
}

char *test_table_all() {
    TEST(test_address);
    TEST(test_string);
    TEST(test_stress);
    TEST(test_remove);
    TEST(test_iterator);
    return 0;
}
