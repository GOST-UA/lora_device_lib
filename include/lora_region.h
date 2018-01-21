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

#if defined(LORA_REGION_EU_863_870)

    #define LORA_CF_LIST    

#elsif defined(LORA_REGION_US_902_928)
#elsif defined(LORA_REGION_CN_779_787)

    #define LORA_CF_LIST

#elsif defined(LORA_REGION_EU_433)

    #define LORA_CF_LIST

#elsif defined(LORA_REGION_AU_915_928)
#elsif defined(LORA_REGION_CN_470_510)
#elsif defined(LORA_REGION_AS_923)

    #define LORA_CF_LIST

#elsif defined(LORA_REGION_KR_920_923)

    #define LORA_CF_LIST

#else

    #define LORA_REGION_ALL
    
#endif
    
enum lora_region_id {
    EU_863_870,
    US_902_928,
    CN_779_787,
    EU_433,
    AU_915_928,
    CN_470_510,
    AS_923,
    KR_920_923
};

struct lora_region_default {

    uint16_t max_fcnt_gap;  /**< maximum frame counter gap */
    
    uint8_t rx1_delay;      /**< rx1 delay (seconds) */    
    uint8_t ja1_delay;      /**< join accept 1 delay (seconds) */
    
    uint8_t rx1_offset;    
    
    uint32_t rx2_freq;      /**< rx2 slot frequency */
    uint8_t rx2_rate;       /**< rx2 slot rate */        
    
    uint8_t adr_ack_limit;  
    uint8_t adr_ack_delay;  
    uint8_t adr_ack_timeout;    
    uint8_t adr_ack_dither;     
    
    uint8_t tx_rate;        /**< an acceptable initial tx data rate */
    uint8_t tx_power;       /**< an acceptable initial power setting */
};

struct lora_data_rate {
    
    enum lora_spreading_factor sf;  /**< spreading factor */
    enum lora_signal_bandwidth bw;  /**< bandwidth */
    uint8_t rate;                   
    uint8_t payload;                /**< maximum payload (accounts for max dwell time) */
};

const struct lora_region *Region_getRegion(enum lora_region_id region);

void Region_getDefaultSettings(const struct lora_region *self, struct lora_region_default *defaults);

/** convert a data rate into parameters for a given region
 *
 * @param[in] region
 * @param[in] rate
 * @param[out] setting
 *
 * @return true if data rate is recognised
 *
 * */
bool Region_getRate(const struct lora_region *self, uint8_t rate, struct lora_data_rate *setting);

bool Region_getBand(const struct lora_region *self, uint32_t freq, uint8_t *band);

bool Region_isDynamic(const struct lora_region *self);

bool Region_getChannel(const struct lora_region *self, uint8_t chIndex, uint32_t *freq, uint8_t *minRate, uint8_t *maxRate);

uint8_t Region_numChannels(const struct lora_region *self);

uint8_t Region_getPayload(const struct lora_region *self, uint8_t rate);

uint8_t Region_getJA1Delay(const struct lora_region *self);

/** Get the default channel settings for this region via an iterator callback
 * 
 * @param[in] region
 * @param[in] receiver passed as first argument of `cb`
 * @param[in] cb this function will be called for each default channel setting
 * 
 * */
void Region_getDefaultChannels(const struct lora_region *self, void *receiver, void (*handler)(void *reciever, uint8_t chIndex, uint32_t freq, uint8_t minRate, uint8_t maxRate));

uint16_t Region_getOffTimeFactor(const struct lora_region *self, uint8_t band);

bool Region_validateRate(const struct lora_region *self, uint8_t chIndex, uint8_t minRate, uint8_t maxRate);
bool Region_validateFreq(const struct lora_region *self, uint8_t chIndex, uint32_t freq);

bool Region_getRX1DataRate(const struct lora_region *self, uint8_t tx_rate, uint8_t rx1_offset, uint8_t *rx1_rate);

bool Region_getRX1Freq(const struct lora_region *self, uint32_t txFreq, uint32_t *freq);

uint16_t Region_getMaxFCNTGap(const struct lora_region *self);

/** derive the rate integer from bandwidth and spreading factor for a given region
 *
 * @note useful for semtech gateway protocol
 *
 * @param[in] region
 * @param[in] sf
 * @param[in] bw
 * @param[out] rate
 *
 * @return true if settings are valid for region
 *
 * */
bool Region_rateFromParameters(const struct lora_region *self, enum lora_spreading_factor sf, enum lora_signal_bandwidth bw, uint8_t *rate);

#ifdef __cplusplus
}
#endif

#endif
