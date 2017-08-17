#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"

#include "lora_mac.h"
#include "lora_channel_list.h"
#include "lora_radio.h"
#include "lora_event.h"

#include <string.h>

static const uint8_t eui[] = "\x00\x00\x00\x00\x00\x00\x00\x00";
static const uint8_t appKey[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

static int setup_MAC(void **user)
{
    static struct lora_mac self;
    static struct lora_channel_list channels;
    static struct lora_radio radio;
    static struct lora_event events;
    
    will_return(ChannelList_region, NULL);
    will_return(Persistent_getAppEUI, eui);
    will_return(Persistent_getDevEUI, eui);
    will_return(Persistent_getAppKey, appKey);
    
    MAC_init(&self, &channels, &radio, &events);
    
    *user = (void *)&self;
    
    return 0;
}

static void test_MAC_init(void **user)
{
    struct lora_mac self;
    struct lora_channel_list channels;
    struct lora_radio radio;
    struct lora_event events;    
    
    will_return(ChannelList_region, NULL);
    will_return(Persistent_getAppEUI, eui);
    will_return(Persistent_getDevEUI, eui);
    will_return(Persistent_getAppKey, appKey);
    
    MAC_init(&self, &channels, &radio, &events);
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


