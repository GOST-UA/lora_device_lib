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
#include "lora_debug.h"

#include <string.h>

bool LDL_init(struct lora_device_lib *self, enum lora_region_id region_id, struct lora_radio *radio)
{
    LORA_PEDANTIC(self != NULL)
    LORA_PEDANTIC(radio != NULL)
    
    bool retval = false;
    
    (void)memset(self, 0, sizeof(*self));
    
    if(Region_getRegion(region_id)){

        MAC_init(&self->mac, region_id, radio);        
        retval = true;        
    }
    else{
        
        LORA_ERROR("region not supported")
    }
    
    return retval;
}

bool LDL_personalize(struct lora_device_lib *self, uint32_t devAddr, const void *nwkSKey, const void *appSKey)
{
    return MAC_personalize(&self->mac, devAddr, nwkSKey, appSKey);
}

#if 0
bool LDL_addChannel(struct lora_device_lib *self, uint32_t freq, uint8_t chIndex)
{
    return Channel_add(&self->channels, freq, chIndex);
}

void LDL_removeChannel(struct lora_device_lib *self, uint8_t chIndex)
{
    Channel_remove(&self->channels, chIndex);
}

bool LDL_maskChannel(struct lora_device_lib *self, uint8_t chIndex)
{
    return Channel_mask(&self->channels, chIndex);
}

void LDL_unmaskChannel(struct lora_device_lib *self, uint8_t chIndex)
{
    Channel_unmask(&self->channels, chIndex);
}
#endif

bool LDL_setRateAndPower(struct lora_device_lib *self, uint8_t rate, uint8_t power)
{
    return false;
}

void LDL_setResponseHandler(struct lora_device_lib *self, void *receiver, lora_mac_response_fn cb)
{
    MAC_setResponseHandler(&self->mac, receiver, cb);
}

bool LDL_join(struct lora_device_lib *self)
{
    return MAC_join(&self->mac);
}

bool LDL_send(struct lora_device_lib *self, bool confirmed, uint8_t port, const void *data, uint8_t len)
{
    return MAC_send(&self->mac, confirmed, port, data, len);
}

void LDL_tick(struct lora_device_lib *self)
{
    Event_tick(&self->events);
}

