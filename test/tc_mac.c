#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"

#include "lora_mac.h"
#include "lora_radio_sx1272.h"
#include "lora_event.h"

#include <string.h>

static const uint8_t eui[] = "\x00\x00\x00\x00\x00\x00\x00\x00";
static const uint8_t appKey[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

static int setup_MAC(void **user)
{
    static struct lora_mac self;
    static struct lora_radio radio;
    
    will_return(System_getAppEUI, eui);
    will_return(System_getDevEUI, eui);
    will_return(System_getAppKey, appKey);
    
    MAC_init(&self, EU_863_870, &radio);
    
    *user = (void *)&self;
    
    return 0;
}

static void test_MAC_init(void **user)
{
    struct lora_mac self;
    struct lora_radio radio;
    
    will_return(System_getAppEUI, eui);
    will_return(System_getDevEUI, eui);
    will_return(System_getAppKey, appKey);
    
    MAC_init(&self, EU_863_870, &radio);
}

static void test_MAC_personalize(void **user)
{
    struct lora_mac *self = (struct lora_mac *)(*user);
    uint32_t devAddr = 0U;
    
    MAC_personalize(self, devAddr, "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa", "\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb");
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_MAC_init),
        cmocka_unit_test_setup(test_MAC_personalize, setup_MAC),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}


