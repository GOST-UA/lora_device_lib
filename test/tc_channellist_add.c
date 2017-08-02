#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"
#include "lora_channel_list.h"
#include "lora_region.h"

struct lora_region {
    
};

static int setup_emptyChannelList(void **user)
{
    static struct lora_channel_list self;
    static struct lora_region region;
    ChannelList_init(&self, &region);
    *user = (void *)&self;
    return 0;
}

static void test_ChannelList_add_badChannel_byCapacity(void **user)
{
    struct lora_channel_list *self = (struct lora_channel_list *)(*user);

    assert_false(ChannelList_add(self, ChannelList_capacity(self), 42U));
}

static void test_ChannelList_add_badChannel_byFrequency(void **user)
{
    struct lora_channel_list *self = (struct lora_channel_list *)(*user);
    
    will_return(Region_validateFrequency, false);

    assert_false(ChannelList_add(self, 0, 42U));
}

static void test_ChannelList_add_goodChannel(void **user)
{
    struct lora_channel_list *self = (struct lora_channel_list *)(*user);
    
    will_return(Region_validateFrequency, true);    // channel ok for region
    will_return(Region_validateFrequency, 0);       // channel in this band index

    assert_true(ChannelList_add(self, 0, 42U));    
}

static void test_ChannelList_add_sameChannel(void **user)
{
    struct lora_channel_list *self = (struct lora_channel_list *)(*user);
    
    will_return(Region_validateFrequency, true);    // channel ok for region
    will_return(Region_validateFrequency, 0);       // channel in this band index
    
    assert_true(ChannelList_add(self, 0, 42U));
    
    will_return(Region_validateFrequency, true);    // channel ok for region
    will_return(Region_validateFrequency, 0);       // channel in this band index
    
    assert_true(ChannelList_add(self, 0, 42U));
}

static void test_ChannelList_add_maxChannels(void **user)
{
    struct lora_channel_list *self = (struct lora_channel_list *)(*user);

    size_t i;
    for(i=0U; i < ChannelList_capacity(self); i++){
    
        will_return(Region_validateFrequency, true);    // channel ok for region
        will_return(Region_validateFrequency, 0);       // channel in this band index

        assert_true(ChannelList_add(self, i, i+1U));   // use counter as non-zero frequency
    }

    assert_false(ChannelList_add(self, i, i+1U));   // use counter as non-zero frequency
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_ChannelList_add_badChannel_byCapacity, setup_emptyChannelList),
        cmocka_unit_test_setup(test_ChannelList_add_badChannel_byFrequency, setup_emptyChannelList),
        cmocka_unit_test_setup(test_ChannelList_add_goodChannel, setup_emptyChannelList),
        cmocka_unit_test_setup(test_ChannelList_add_sameChannel, setup_emptyChannelList),
        cmocka_unit_test_setup(test_ChannelList_add_maxChannels, setup_emptyChannelList),
    };
    
    return cmocka_run_group_tests(tests, NULL, NULL);
}
