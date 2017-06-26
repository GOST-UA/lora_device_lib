#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"

#include "lora_mac.h"

#include <string.h>

struct lora_channel_list {
    
};

struct lora_radio {
        
};

struct lora_event {
    
};

static int setup_LoraMAC(void **user)
{
    static struct lora_mac self;
    static struct lora_channel_list channels;
    static struct lora_radio radio;
    static struct lora_event events;    
    
    LoraMAC_init(&self, &channels, &radio, &events);
    
    *user = (void *)&self;
    
    return 0;
}

static void test_LoraMAC_setSession(void **user)
{
    struct lora_mac *self = (struct lora_mac *)(*user);
    
    LoraMAC_setSession(self, 1, "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa", "\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb");
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_LoraMAC_setSession, setup_LoraMAC),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}


