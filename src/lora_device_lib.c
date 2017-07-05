/* Copyright (c) 2017 Cameron Harper
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * */

#include "lora_device_lib.h"

struct ldl ldl_new(enum lora_region_id region, enum lora_radio_type radioType, const struct lora_board *board)
{
    struct ldl self;
    
    (void)memset(self, 0, sizeof(self));

    ChannelList_init(&self.channels, region);
    ChannelSchedule_init(&self.events);
    LoraRadio_init(&self.radio, board, radioType);
    LoraMAC_init(&self.mac, &self.channels, &self.radio, self.region, &self.events);
}

bool ldl_personalize(struct ldl *self, uint32_t devAddr, const void *nwkSKey, const void *appSKey)
{
    return LoraMAC_personalize(&self->mac, devAddr, nwkSKey, appSKey);
}

bool ldl_addChannel(struct ldl *self, uint32_t freq, uint8_t chIndex)
{
    return ChannelList_add(&self->channels, freq, chIndex);
}

void ldl_removeChannel(struct ldl *self, uint8_t chIndex)
{
    ChannelList_remove(&self->channels, chIndex);
}

bool ldl_maskChannel(struct ldl *self, uint8_t chIndex)
{
    return ChannelList_mask(&self->channels, chIndex);
}

void ldl_unmaskChannel(struct ldl *self, uint8_t chIndex)
{
    ChannelList_unmask(&self->channels, chIndex);
}

bool ldl_setRateAndPower(struct ldl *self, uint8_t rate, uint8_t power)
{
    return ChannelList_mask(&self->channels, rate, power);
}

void ldl_setReceiveHandler(struct ldl *self, void *receiver, rxCompleteCB cb)
{
    LoraMAC_setReceiveHandler(&self->mac, receiver, cb);
}

void ldl_setTransmitHandler(struct ldl *self, void *receiver, txCompleteCB cb)
{
    LoraMAC_setTransmitHandler(&self->mac, receiver, cb);
}

void ldl_setJoinHandler(struct ldl *self, void *receiver, txCompleteCB cb)
{
    LoraMAC_setJoinHandler(&self->mac, receiver, cb);
}

bool ldl_join(struct ldl *self, void *receiver, joinConfirmation cb);
{
    return LoraMAC_join(&self->mac);
}

bool ldl_send(struct ldl *self, bool confirmed, uint8_t port, const void *data, uint8_t len)
{
    return LoraMAC_send(&self->mac, confirmed, port, data, len);
}

void ldl_tick(struct ldl *self);
{
    LoraEvent_tick(&self->events);
}
