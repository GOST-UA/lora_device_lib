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
#include "lora_radio.h"
#include "lora_region.h"

struct lora_device_lib {
    
    struct lora_mac mac;
    struct lora_event events;    
};


bool LDL_init(struct lora_device_lib *self, enum lora_region_id region_id, struct lora_radio *radio);

bool LDL_personalize(struct lora_device_lib *self, uint32_t devAddr, const void *nwkSKey, const void *appSKey);

bool LDL_addChannel(struct lora_device_lib *self, uint32_t freq, uint8_t chIndex);
void LDL_removeChannel(struct lora_device_lib *self, uint8_t chIndex);
bool LDL_maskChannel(struct lora_device_lib *self, uint8_t chIndex);
void LDL_unmaskChannel(struct lora_device_lib *self, uint8_t chIndex);

bool LDL_setRateAndPower(struct lora_device_lib *self, uint8_t rate, uint8_t power);

void LDL_setResponseHandler(struct lora_device_lib *self, void *receiver, lora_mac_response_fn cb);

bool LDL_join(struct lora_device_lib *self);
bool LDL_send(struct lora_device_lib *self, bool confirmed, uint8_t port, const void *data, uint8_t len);

void LDL_tick(struct lora_device_lib *self);

#ifdef __cplusplus
}
#endif

#endif
