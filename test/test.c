#include "test.h"
#include "test_array.h"
#include "test_set.h"
#include "test_table.h"
#include "test_uint_table.h"

int tests_success = 0;
int tests_fail = 0;
int tests_count = 0;

int main() {
    printf("\n");
    TEST_SET(test_array_all);
    TEST_SET(test_table_all);
    TEST_SET(test_uint_table_all);
    TEST_SET(test_set_all);
    printf("Success: %d, Failed: %d, Total: %d\n\n", tests_success, tests_fail, tests_count);
    return 0;
}
