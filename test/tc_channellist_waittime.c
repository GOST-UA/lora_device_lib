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

static int setup_singleChannelList(void **user)
{
    static struct lora_channel_list self;
    static struct lora_region region;
    ChannelList_init(&self, &region);
    *user = (void *)&self;
        
    will_return(Region_validateFrequency, true);    // channel ok for region
    will_return(Region_validateFrequency, 0);       // channel in this band index

    if(ChannelList_add(&self, 0, 1)){
    
        *user = (void *)&self;

#if  0
        static const struct lora_data_rate dummy = {
            .bw = BW_125,
            .sf = SF_8,
            .payload = 42
        };
    
        //will_return_always(Region_getDataRateParameters, &dummy);
#endif
        return 0;
    }
    else{
        
        return -1;
    }
}

void test_ChannelList_waitTime_noChannel(void **user)
{
    struct lora_channel_list *self = (struct lora_channel_list *)(*user);

    // no waiting
    assert_int_equal(0, ChannelList_waitTime(self, 0U));    
    assert_int_equal(0, ChannelList_waitTime(self, 1234U));

    // no channel
    //assert_int_equal(0, ChannelList_frequency(self));    
}

void test_ChannelList_waitTime_fresh(void **user)
{
    struct lora_channel_list *self = (struct lora_channel_list *)(*user);

    // no waiting
    assert_int_equal(0, ChannelList_waitTime(self, 0U));
    assert_int_equal(0, ChannelList_waitTime(self, 1234U));

    // channel "1Hz"
    //assert_int_equal(1, ChannelList_frequency(self));    
}

void test_ChannelList_waitTime_use(void **user)
{
    struct lora_channel_list *self = (struct lora_channel_list *)(*user);

    will_return(Region_getOffTimeFactor, true);
    will_return(Region_getOffTimeFactor, 1000); // 1% duty cycle

    assert_int_equal(0, ChannelList_waitTime(self, 0U));
    
    ChannelList_registerTransmission(self, 0, 1500000);          // say we tx for 1.5 seconds
    
    uint64_t waitTime = ChannelList_waitTime(self, 0U);

    assert_true(waitTime > 0U);  // single channel will not be available right now

    //assert_int_equal(1, ChannelList_frequency(self));      // channel "1Hz"
}



int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_ChannelList_waitTime_noChannel, setup_emptyChannelList),
        cmocka_unit_test_setup(test_ChannelList_waitTime_fresh, setup_singleChannelList),        
        cmocka_unit_test_setup(test_ChannelList_waitTime_use, setup_singleChannelList),

    };
    
    return cmocka_run_group_tests(tests, NULL, NULL);
}
