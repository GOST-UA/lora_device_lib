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

#ifndef LORA_DEVICE_LIB_H
#define LORA_DEVICE_LIB_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lora_mac.h"
#include "lora_event.h"
#include "lora_channel_list.h"
#include "lora_radio.h"
#include "lora_region.h"

struct ldl {
    
    struct lora_mac mac;
    struct lora_channel_list channels;
    struct lora_event events;    
};


bool ldl_init(struct ldl *self, enum lora_region_id region_id, struct lora_radio *radio);

bool ldl_personalize(struct ldl *self, uint32_t devAddr, const void *nwkSKey, const void *appSKey);

bool ldl_addChannel(struct ldl *self, uint32_t freq, uint8_t chIndex);
void ldl_removeChannel(struct ldl *self, uint8_t chIndex);
bool ldl_maskChannel(struct ldl *self, uint8_t chIndex);
void ldl_unmaskChannel(struct ldl *self, uint8_t chIndex);

bool ldl_setRateAndPower(struct ldl *self, uint8_t rate, uint8_t power);

void ldl_setResponseHandler(struct ldl *self, void *receiver, lora_mac_response_fn cb);

bool ldl_join(struct ldl *self);
bool ldl_send(struct ldl *self, bool confirmed, uint8_t port, const void *data, uint8_t len);

void ldl_tick(struct ldl *self);

#ifdef __cplusplus
}
#endif

#endif
