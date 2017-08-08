#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"

#include "lora_device_lib.h"

static void test_init(void **user)
{
    struct ldl self;
    struct lora_board board;
    
    ldl_init(&self, EU_863_870, &board);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_init),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
