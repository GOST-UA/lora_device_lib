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
    return false;
}

static int setup_emptyChannelList(void **user)
{
    static struct lora_channel_list self;
    ChannelList_init(&self, US_902_928);    // the US will not add default channels
    *user = (void *)&self;
    return 0;
}

static void test_ChannelList_add_badChannel(void **user)
{
    struct lora_channel_list *self = (struct lora_channel_list *)(*user);
    
    will_return(LoraRegion_validateFrequency, false);

    assert_false(ChannelList_add(self, 42U));
}

static void test_ChannelList_add_zeroChannel(void **user)
{
    struct lora_channel_list *self = (struct lora_channel_list *)(*user);
    
    assert_false(ChannelList_add(self, 0U));
}

static void test_ChannelList_add_goodChannel(void **user)
{
    struct lora_channel_list *self = (struct lora_channel_list *)(*user);
    
    will_return(LoraRegion_validateFrequency, true);    // channel ok for region
    will_return(LoraRegion_validateFrequency, 0);       // channel in this band index

    assert_true(ChannelList_add(self, 42U));    
}

static void test_ChannelList_add_sameChannel(void **user)
{
    struct lora_channel_list *self = (struct lora_channel_list *)(*user);
    
    will_return(LoraRegion_validateFrequency, true);    // channel ok for region
    will_return(LoraRegion_validateFrequency, 0);       // channel in this band index
    
    assert_int_equal(0, ChannelList_size(self));
    assert_true(ChannelList_add(self, 42U));
    assert_int_equal(1, ChannelList_size(self));    
    assert_true(ChannelList_add(self, 42U));
    assert_int_equal(1, ChannelList_size(self));
}

static void test_ChannelList_add_maxChannels(void **user)
{
    struct lora_channel_list *self = (struct lora_channel_list *)(*user);

    size_t i;
    for(i=0U; i < ChannelList_capacity(self); i++){
    
        will_return(LoraRegion_validateFrequency, true);    // channel ok for region
        will_return(LoraRegion_validateFrequency, 0);       // channel in this band index

        assert_true(ChannelList_add(self, i+1U));   // use counter as non-zero frequency
    }

    assert_int_equal(ChannelList_capacity(self), ChannelList_size(self));

    assert_false(ChannelList_add(self, i+1U));   // use counter as non-zero frequency
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_ChannelList_add_badChannel, setup_emptyChannelList),
        cmocka_unit_test_setup(test_ChannelList_add_zeroChannel, setup_emptyChannelList),
        cmocka_unit_test_setup(test_ChannelList_add_goodChannel, setup_emptyChannelList),
        cmocka_unit_test_setup(test_ChannelList_add_sameChannel, setup_emptyChannelList),
        cmocka_unit_test_setup(test_ChannelList_add_maxChannels, setup_emptyChannelList),
    };
    
    return cmocka_run_group_tests(tests, NULL, NULL);
}
