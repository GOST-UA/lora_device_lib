#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"
#include "lora_channel_list.h"
#include "lora_region.h"

const struct data_rate *LoraRegion_getDataRateParameters(enum lora_region_id region, uint8_t rate)
{
    return mock_ptr_type(struct data_rate *);
}

bool LoraRegion_validateFrequency(enum lora_region_id region, uint32_t frequency, uint8_t *band)
{
    bool retval = false;
    
    if(mock_type(bool)){

        *band = mock();
        retval = true;
    }

    return retval;
}

bool LoraRegion_getOffTimeFactor(enum lora_region_id region, uint8_t band, uint16_t *offTimeFactor)
{
    bool retval = mock_type(bool);
    if(retval){

        *offTimeFactor = mock_type(uint16_t);
    }
    return retval;
}

static int setup_emptyChannelList(void **user)
{
    static struct lora_channel_list self;
    ChannelList_init(&self, US_902_928);    // the US will not add default channels
    
    *user = (void *)&self;
    return 0;
}

static int setup_singleChannelList(void **user)
{
    static struct lora_channel_list self;
    ChannelList_init(&self, US_902_928);    // the US will not add default channels

    will_return(LoraRegion_validateFrequency, true);    // channel ok for region
    will_return(LoraRegion_validateFrequency, 0);       // channel in this band index

    if(ChannelList_add(&self, 1)){
    
        *user = (void *)&self;

        static const struct data_rate dummy = {
            .bw = BW_125,
            .sf = SF_8,
            .payload = 42
        };
    
        will_return_always(LoraRegion_getDataRateParameters, &dummy);

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
    assert_int_equal(0, ChannelList_frequency(self));    
}

void test_ChannelList_waitTime_fresh(void **user)
{
    struct lora_channel_list *self = (struct lora_channel_list *)(*user);

    // no waiting
    assert_int_equal(0, ChannelList_waitTime(self, 0U));
    assert_int_equal(0, ChannelList_waitTime(self, 1234U));

    // channel "1Hz"
    assert_int_equal(1, ChannelList_frequency(self));    
}

void test_ChannelList_waitTime_use(void **user)
{
    struct lora_channel_list *self = (struct lora_channel_list *)(*user);

    will_return(LoraRegion_getOffTimeFactor, true);
    will_return(LoraRegion_getOffTimeFactor, 1000); // 1% duty cycle

    assert_int_equal(0, ChannelList_waitTime(self, 0U));
    
    ChannelList_registerTransmission(self, 0, 10);          // say we sent 10 bytes
    
    uint32_t timeAvailable = ChannelList_waitTime(self, 0U);

    assert_true(timeAvailable > 0U);  // single channel will not be available right now

    assert_int_equal(1, ChannelList_frequency(self));      // channel "1Hz"
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
