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

#ifndef LORA_CHANNEL_LIST_H
#define LORA_CHANNEL_LIST_H

#include "lora_radio_defs.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifndef LORA_NUM_CHANNELS
    #define LORA_NUM_CHANNELS    16U
#endif

#ifndef LORA_NUM_BANDS
    #define LORA_NUM_BANDS       6U
#endif

struct lora_channel {
    uint32_t freq;
    int band;
    bool masked;
    uint8_t minRate;
    uint8_t maxRate;    // todo: compact
};

struct lora_band {
    uint64_t timeReady;     ///< system time (at or after) this band will be available
    int channel;            ///< channels are cycled round robin per band so we need a reference to channel
    int numUnmasked;        ///< number of channels in this band
};

struct lora_channel_list {

    struct lora_channel channels[LORA_NUM_CHANNELS];
    struct lora_band bands[LORA_NUM_BANDS];

    const struct lora_region *region;
    
    uint8_t rx1_offset;
    uint32_t rx2_freq;
    uint8_t  rx2_rate;
    uint8_t rate;
    
    int nextChannel;
    int nextBand;

    int numUnmasked;    ///< total number of unmasked channels
};

struct lora_channel_setting {
  
    uint32_t freq;
  
    const uint8_t *rates;
    uint8_t numRates;
  
    uint8_t minRate;  
    uint8_t maxRate;    
    
    uint8_t chIndex;
};

struct lora_channel_rx_setting {
  
    uint32_t freq;
    uint8_t chIndex;
};

void ChannelList_init(struct lora_channel_list *self, const struct lora_region *region);

/** Add a (transmission) channel
 * 
 * @param[in] self
 * @param[in] chIndex   channel index (0..LORA_NUM_CHANNELS-1)
 * @param[in] freq      frequency in Hz (0 has same effect as remove)
 * 
 * @return true if channel could be added
 * 
 * */
bool ChannelList_add(struct lora_channel_list *self, uint8_t chIndex, uint32_t freq);

/** Remove a (transmission) channel
 * 
 * @param[in] self
 * @param[in] chIndex channel index (0..LORA_NUM_CHANNELS)
 * 
 * */
void ChannelList_remove(struct lora_channel_list *self, uint8_t chIndex);

bool ChannelList_mask(struct lora_channel_list *self, uint8_t chIndex);

void ChannelList_unmask(struct lora_channel_list *self, uint8_t chIndex);

uint64_t ChannelList_waitTime(const struct lora_channel_list *self, uint64_t timeNow);

void ChannelList_registerTransmission(struct lora_channel_list *self, uint64_t time, uint32_t airTime);

size_t ChannelList_capacity(const struct lora_channel_list *self);

bool ChannelList_txSetting(const struct lora_channel_list *self, struct lora_channel_setting *setting);
void ChannelList_rx2Setting(const struct lora_channel_list *self, uint32_t *freq, uint8_t *rate);

const struct lora_region *ChannelList_region(const struct lora_channel_list *self);

bool ChannelList_constrainRate(struct lora_channel_list *self, uint8_t chIndex, uint8_t minRate, uint8_t maxRate);

#endif
