#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"

#include "lora_mac.h"
#include "lora_radio_sx1272.h"

#include "mock_lora_system.h"

#include <string.h>

static const uint8_t eui[] = "\x00\x00\x00\x00\x00\x00\x00\x00";
static const uint8_t key[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

static int setup_MAC(void **user)
{
    static struct lora_mac self;
    static struct lora_radio radio;
    static struct mock_system_param params;
        
    mock_lora_system_init(&params);
    
    MAC_init(&self, &params, EU_863_870, &radio);
    
    MAC_restoreDefaults(&self);
    
    *user = (void *)&self;
    
    return 0;
}

static void test_MAC_init(void **user)
{
    setup_MAC(user);
}

static void test_MAC_personalize(void **user)
{
    struct lora_mac *self = (struct lora_mac *)(*user);
    uint32_t devAddr = 0U;
    
    bool retval = MAC_personalize(self, devAddr, "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa", "\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb");
    
    assert_true(retval);
}

static void test_MAC_join_responseHandler(void *receiver, enum lora_mac_response_type type, const union lora_mac_response_arg *arg)
{
}

static void test_MAC_join(void **user)
{
    struct lora_mac *self = (struct lora_mac *)(*user);
    bool retval;
    
    MAC_setResponseHandler(self, NULL, test_MAC_join_responseHandler);
    
    retval = MAC_join(self);
    
    assert_true(retval);
    
    will_return(Radio_transmit, true);
    
    assert_true(MAC_timeUntilNextEvent(self) == UINT64_MAX);
    
    MAC_tick(self);    
    
    assert_true(MAC_timeUntilNextEvent(self) > 0U && MAC_timeUntilNextEvent(self) != UINT64_MAX);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_MAC_init),
        cmocka_unit_test_setup(test_MAC_personalize, setup_MAC),
        cmocka_unit_test_setup(test_MAC_join, setup_MAC),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
