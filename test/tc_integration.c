#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"

#include "lora_device_lib.h"
#include "lora_radio_sx1272.h"

static const uint8_t eui[] = "\x00\x00\x00\x00\x00\x00\x00\x00";
static const uint8_t appKey[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

static void test_init(void **user)
{
    struct lora_device_lib self;
    struct lora_board board;
    struct lora_radio radio;

    Radio_init(&radio, &board);

    will_return(System_getAppEUI, eui);
    will_return(System_getDevEUI, eui);
    will_return(System_getAppKey, appKey);
    
    LDL_init(&self, EU_863_870, &radio);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_init),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
