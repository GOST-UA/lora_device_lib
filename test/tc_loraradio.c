#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"

#include "lora_radio.h"

#include <string.h>

static void test_LoraRadio_init(void **user)
{
}

void LoraMAC_eventTXComplete(struct lora_mac *self)
{
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_LoraRadio_init),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}

