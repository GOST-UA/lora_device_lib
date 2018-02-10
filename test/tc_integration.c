#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"

#include "lora_mac.h"
#include "lora_radio_sx1272.h"

static const uint8_t eui[] = "\x00\x00\x00\x00\x00\x00\x00\x00";
static const uint8_t key[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

static void responseHandler(void *receiver, enum lora_mac_response_type type, const union lora_mac_response_arg *arg)
{
}

static void test_init(void **user)
{
    struct lora_mac self;
    struct lora_board board;
    struct lora_radio radio;

    Radio_init(&radio, &board);

    MAC_init(&self, NULL, EU_863_870, &radio, NULL, responseHandler);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_init),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
