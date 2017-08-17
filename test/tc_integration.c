#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"

#include "lora_device_lib.h"

static const uint8_t eui[] = "\x00\x00\x00\x00\x00\x00\x00\x00";
static const uint8_t appKey[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

static void test_init(void **user)
{
    struct ldl self;
    struct lora_board board;

    will_return(Persistent_getAppEUI, eui);
    will_return(Persistent_getDevEUI, eui);
    will_return(Persistent_getAppKey, appKey);
    
    ldl_init(&self, EU_863_870, &board);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_init),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
