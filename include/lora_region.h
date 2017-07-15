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

#ifndef LORA_REGION_H
#define LORA_REGION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lora_radio_defs.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

enum lora_region_id {
    EU_863_870,
    US_902_928,
    CN_779_787,
    EU_433,
    AUSTRALIA_915_928,
    CN_470_510,
    AS_923,
    KR_920_923
};

struct lora_region_default {

    uint16_t max_fcnt_gap;  /**< maximum frame counter gap */
    
    uint8_t rx1_delay;      /**< rx1 delay (seconds) */
    uint8_t rx2_delay;      /**< rx2 delay (seconds) */
    
    uint8_t ja1_delay;      /**< join accept 1 delay (seconds) */
    uint8_t ja2_delay;      /**< join accept 2 delay (seconds) */
    
    uint8_t adr_ack_limit;  
    uint8_t adr_ack_delay;  
    uint8_t adr_ack_timeout;    
    uint8_t adr_ack_dither;     
    
    uint32_t rx2_freq;      /**< rx2 slot frequency */
    uint8_t rx2_rate;       /**< rx2 slot rate */        
    
    uint8_t init_tx_rate;        /**< an acceptable initial tx data rate */
    uint8_t init_tx_power;       /**< an acceptable initial power setting */
};

/** LoRa Data Rate is a function of spreading factor and bandwidth
 *
 * */
struct lora_data_rate {
    enum lora_spreading_factor sf;  /**< spreading factor */
    enum lora_signal_bandwidth bw;  /**< bandwidth */
    uint16_t payload;               /**< maximum payload (accounts for max dwell time) */
};

struct lora_region;

/**
 * @param[in] region region id
 * @return pointer to region structure
 * @retval NULL region id not supported
 * 
 * */
const struct lora_region *Region_getRegion(enum lora_region_id region);

/** Retrieve datarate parameters for a given DR integer and region
 *
 * @param[in] self
 * @param[in] rate DRn integer (e.g. 0 == DR0)
 *
 * @return data rate parameter structure
 *
 * @retval NULL data rate not recognised 
 *
 * */
const struct lora_data_rate *Region_getDataRateParameters(const struct lora_region *self, uint8_t rate);

/** Test that frequency is acceptable for region
 *
 * @note will also return the frequency band index useful for determining
 *       duty cycle constraints
 *
 * @param[in] self
 * @param[in] frequency
 * @param[out] band     band this frequency belongs to
 *
 * @return true if frequency within bounds
 *
 * */
bool Region_validateFrequency(const struct lora_region *self, uint32_t frequency, uint8_t *band);

/** Get the off-time factor for a given channel band
 * 
 * @param[in] self
 * @param[in] band channel band
 * @param[out] offtime_factor
 * 
 * @return true if off-time factor exists
 * 
 * */
bool Region_getOffTimeFactor(const struct lora_region *self, uint8_t band, uint16_t *offtime_factor);

/** Get default setting structure
 * 
 * @see lora_region_default
 * 
 * @param[in] self
 * @return defaults structure
 * 
 * */
const struct lora_region_default *Region_getDefaultSettings(const struct lora_region *self);
    
/** Get the default channel settings for this region via an iterator callback
 * 
 * @param[in] self
 * @param[in] receiver passed as first argument of `cb`
 * @param[in] cb this function will be called for each default channel setting
 * 
 * */
void Region_getDefaultChannels(const struct lora_region *self, void *receiver, void (*cb)(void *reciever, uint8_t chIndex, uint32_t freq));

/** Get the RX1 data rate for a given tx data rate and rx1 offset setting
 * 
 * @param[in] self
 * @param[in] tx_rate
 * @param[in] rx1_offset
 * @param[out] rx1_rate
 * 
 * @return true if RX1 data rate exists
 * 
 * */
bool Region_getRX1DataRate(const struct lora_region *self, uint8_t tx_rate, uint8_t rx1_offset, uint8_t *rx1_rate);

#ifdef __cplusplus
}
#endif

#endif
