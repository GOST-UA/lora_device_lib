#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"

#include "lora_region.h"

static void eu_868_870_is_supported(void **user)
{
    assert_true(Region_supported(EU_863_870));
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(eu_868_870_is_supported),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
