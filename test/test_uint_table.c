#include "test_uint_table.h"

static char *test_small() {

    UIntTable *tab = new_uint_table();

    uint_table_put(tab, 4, "foo");
    uint_table_put(tab, 6, "bar");
    uint_table_put(tab, 8, "baz");

    ASSERT("size", uint_table_size(tab) == 3);

    ASSERT("get(1)", strcmp(uint_table_get(tab, 6), "bar") == 0);
    ASSERT("get(2)", strcmp(uint_table_get(tab, 8), "baz") == 0);
    ASSERT("get(3)", strcmp(uint_table_get(tab, 4), "foo") == 0);

    ASSERT("has(1)", uint_table_has(tab, 4));
    ASSERT("has(2)", uint_table_has(tab, 6));
    ASSERT("has(3)", uint_table_has(tab, 8));

    uint_table_remove(tab, 4);
    uint_table_remove(tab, 6);
    uint_table_remove(tab, 8);

    ASSERT("size == 0", uint_table_size(tab) == 0);

    ASSERT("!has(1)", !uint_table_has(tab, 4));
    ASSERT("!has(2)", !uint_table_has(tab, 6));
    ASSERT("!has(3)", !uint_table_has(tab, 8));

    return 0;
}

char *test_uint_table_all() {
    TEST(test_small);
    return 0;
}
