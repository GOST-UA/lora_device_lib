#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"

#include "lora_region.h"

static void test_Region_getRegion(void **user)
{
    assert_non_null(Region_getRegion(EU_863_870));
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_Region_getRegion),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
