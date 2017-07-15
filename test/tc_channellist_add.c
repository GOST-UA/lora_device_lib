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

void LoraRegion_getDefaultChannels(enum lora_region_id region, void *receiver, void (*handler)(void *reciever, uint8_t chIndex, uint32_t freq))
{
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
    ChannelList_init(&self, EU_863_870);
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
    
    will_return(LoraRegion_validateFrequency, false);

    assert_false(ChannelList_add(self, 0, 42U));
}

static void test_ChannelList_add_goodChannel(void **user)
{
    struct lora_channel_list *self = (struct lora_channel_list *)(*user);
    
    will_return(LoraRegion_validateFrequency, true);    // channel ok for region
    will_return(LoraRegion_validateFrequency, 0);       // channel in this band index

    assert_true(ChannelList_add(self, 0, 42U));    
}

static void test_ChannelList_add_sameChannel(void **user)
{
    struct lora_channel_list *self = (struct lora_channel_list *)(*user);
    
    will_return(LoraRegion_validateFrequency, true);    // channel ok for region
    will_return(LoraRegion_validateFrequency, 0);       // channel in this band index
    
    assert_true(ChannelList_add(self, 0, 42U));
    
    will_return(LoraRegion_validateFrequency, true);    // channel ok for region
    will_return(LoraRegion_validateFrequency, 0);       // channel in this band index
    
    assert_true(ChannelList_add(self, 0, 42U));
}

static void test_ChannelList_add_maxChannels(void **user)
{
    struct lora_channel_list *self = (struct lora_channel_list *)(*user);

    size_t i;
    for(i=0U; i < ChannelList_capacity(self); i++){
    
        will_return(LoraRegion_validateFrequency, true);    // channel ok for region
        will_return(LoraRegion_validateFrequency, 0);       // channel in this band index

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
