#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdbool.h>

#include "cmocka.h"
#include "lora_channel_list.h"

void ChannelList_init(struct lora_channel_list *self, const struct lora_region *region)
{
}

bool ChannelList_add(struct lora_channel_list *self, uint8_t chIndex, uint32_t freq)
{
    return mock();
}

void ChannelList_remove(struct lora_channel_list *self, uint8_t chIndex)
{
}

bool ChannelList_mask(struct lora_channel_list *self, uint8_t chIndex)
{
    return mock();
}

void ChannelList_unmask(struct lora_channel_list *self, uint8_t chIndex)
{
}

uint64_t ChannelList_waitTime(const struct lora_channel_list *self, uint64_t timeNow)
{
    return mock();
}

void ChannelList_registerTransmission(struct lora_channel_list *self, uint64_t time, uint32_t airTime)
{
}

size_t ChannelList_capacity(const struct lora_channel_list *self)
{
    return mock();
}

bool ChannelList_txSetting(const struct lora_channel_list *self, struct lora_channel_setting *setting)
{
    
    return mock();
}

void ChannelList_rx2Setting(const struct lora_channel_list *self, uint32_t *freq, uint8_t *rate)
{
    *freq = mock();
    *rate = mock();
}

const struct lora_region *ChannelList_region(const struct lora_channel_list *self)
{
    return mock_ptr_type(const struct lora_region *);
}

bool ChannelList_constrainRate(struct lora_channel_list *self, uint8_t chIndex, uint8_t minRate, uint8_t maxRate)
{
    return mock();
}
