/* Copyright (c) 2017-2018 Cameron Harper
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
    
enum lora_region {
    EU_863_870,
    US_902_928,
    CN_779_787,
    EU_433,
    AU_915_928,
    CN_470_510,
    AS_923,
    KR_920_923,
    IN_865_867
};

bool Region_supported(enum lora_region region);

bool Region_getRate(enum lora_region region, uint8_t rate, enum lora_spreading_factor *sf, enum lora_signal_bandwidth *bw);
bool Region_getPayload(enum lora_region region, uint8_t rate, uint8_t *payload);

bool Region_getBand(enum lora_region region, uint32_t freq, uint8_t *band);

bool Region_isDynamic(enum lora_region region);

bool Region_getChannel(enum lora_region region, uint8_t chIndex, uint32_t *freq, uint8_t *minRate, uint8_t *maxRate);

uint8_t Region_numChannels(enum lora_region region);

uint8_t Region_getJA1Delay(enum lora_region region);

/** Get the default channel settings for this region via an iterator callback
 * 
 * @param[in] region
 * @param[in] receiver passed as first argument of `cb`
 * @param[in] cb this function will be called for each default channel setting
 * 
 * */
void Region_getDefaultChannels(enum lora_region region, void *receiver, void (*handler)(void *reciever, uint8_t chIndex, uint32_t freq, uint8_t minRate, uint8_t maxRate));

uint16_t Region_getOffTimeFactor(enum lora_region region, uint8_t band);

bool Region_validateRate(enum lora_region region, uint8_t chIndex, uint8_t minRate, uint8_t maxRate);
bool Region_validateFreq(enum lora_region region, uint8_t chIndex, uint32_t freq);

bool Region_getRX1DataRate(enum lora_region region, uint8_t tx_rate, uint8_t rx1_offset, uint8_t *rx1_rate);

bool Region_getRX1Freq(enum lora_region region, uint32_t txFreq, uint32_t *freq);

uint16_t Region_getMaxFCNTGap(enum lora_region region);

uint8_t Region_getRX1Delay(enum lora_region region);

uint8_t Region_getRX1Offset(enum lora_region region);
uint32_t Region_getRX2Freq(enum lora_region region);
uint8_t Region_getRX2Rate(enum lora_region region);
uint8_t Region_getADRAckLimit(enum lora_region region);
uint8_t Region_getADRAckDelay(enum lora_region region);
uint8_t Region_getADRAckTimeout(enum lora_region region);
uint8_t Region_getADRAckDither(enum lora_region region);
uint8_t Region_getTXRate(enum lora_region region);
uint8_t Region_getTXPower(enum lora_region region);

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
bool Region_rateFromParameters(enum lora_region region, enum lora_spreading_factor sf, enum lora_signal_bandwidth bw, uint8_t *rate);

#ifdef __cplusplus
}
#endif

#endif
