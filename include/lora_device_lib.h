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

#include "lora_mac.h"
#include "lora_schedule.h"
#include "lora_channel_list.h"
#include "lora_radio.h"

struct lora_device_library {
    
    struct lora_mac mac;
    struct lora_channel_list channels;
    struct lora_schedule schedule;
    struct lora_radio radio;    
};

struct lora_mac_init {

    enum lora_region_id region;
    enum lora_radio_type radioType;
    struct lora_radio radio;
    struct lora_schedule schedule;
    struct lora_channel_list channels;
    const struct lora_board *board;
};

void LoraDeviceLib_init(struct lora_device_library *self, enum lora_region_id region, enum lora_radio_type radioType, const struct lora_board *board);

#endif
