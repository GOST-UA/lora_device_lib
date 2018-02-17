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

#include "lora_region.h"
#include "lora_debug.h"

#if defined(LORA_AVR)

    #include <avr/pgmspace.h>
    
#else

    #include <string.h>
    
    #define PROGMEM
    #define memcpy_P memcpy
    
#endif

/* static function prototypes *****************************************/

static bool upRateRange(enum lora_region region, uint8_t chIndex, uint8_t *minRate, uint8_t *maxRate);
static bool freqToChannel(enum lora_region region, uint32_t freq, uint8_t *chIndex);
static bool getRateAndPayload(enum lora_region region, uint8_t rate, enum lora_spreading_factor *sf, enum lora_signal_bandwidth *bw, uint8_t *payload);

/* functions **********************************************************/

bool Region_supported(enum lora_region region)
{
    bool retval;
    
    switch(region){
    case EU_863_870:
    case US_902_928:                    
    case AU_915_928:
        retval = true;
        break;
    case EU_433:
    case AS_923:
    case KR_920_923:
    case CN_779_787:                            
    case CN_470_510:
    case IN_865_867:
    default:                    
        retval = false;                    
        break;
    }
    
    return retval;
}

bool Region_getRate(enum lora_region region, uint8_t rate, enum lora_spreading_factor *sf, enum lora_signal_bandwidth *bw)
{
    uint8_t payload;    
    
    return getRateAndPayload(region, rate, sf, bw, &payload);
}

bool Region_getPayload(enum lora_region region, uint8_t rate, uint8_t *payload)
{
    enum lora_spreading_factor sf; 
    enum lora_signal_bandwidth bw;
    
    return getRateAndPayload(region, rate, &sf, &bw, payload);
}

bool Region_getBand(enum lora_region region, uint32_t freq, uint8_t *band)
{
    LORA_PEDANTIC(band != NULL)

    bool retval = false;

    switch(region){
    case EU_863_870:
    
        retval = true;
    
        if((freq > 863000000U) && (freq < 865000000U)){
            
            *band = 2U;
        }
        else if((freq > 865000000U) && (freq < 868000000U)){
            
            *band = 0U;
        }
        else if((freq > 868000000U) && (freq < 868600000U)){
            
            *band = 1U;
        }
        else if((freq > 868700000U) && (freq < 869200000U)){
            
            *band = 2U;
        }
        else if((freq > 869400000U) && (freq < 869650000U)){
            
            *band = 3U;
        }
        else if((freq > 869700000U) && (freq < 867050000U)){
            
            *band = 4U;
        }
        else{
            
            retval = false;
        }        
        break;
    
    
    case US_902_928:            
    case AU_915_928:
        *band = 0U;
        retval = true;
        break;        
    case EU_433:
    case AS_923:
    case KR_920_923:
    case CN_779_787:                         
    case IN_865_867:
    default:
        break;
    }
    
    return retval;
}

bool Region_isDynamic(enum lora_region region)
{
    bool retval;
    
    switch(region){
    case EU_863_870:
    case EU_433:
    case AS_923:
    case KR_920_923:
    case CN_779_787:
    case IN_865_867:                        
        retval = true;
        break;    
    default:                    
    case US_902_928:                    
    case AU_915_928:
    case CN_470_510:
        retval = false;                    
        break;
    }
    
    return retval;
}

bool Region_getChannel(enum lora_region region, uint8_t chIndex, uint32_t *freq, uint8_t *minRate, uint8_t *maxRate)
{
    bool retval = false;
    
    switch(region){
    default:
    case EU_863_870:
    case EU_433:
    case AS_923:
    case KR_920_923:
    case CN_779_787:  
    case IN_865_867:
        break;                      
    case US_902_928:            
    case AU_915_928:
    
        retval = true;
        
        if(chIndex < 64U){
            
            *freq = 902300000U + ( 200000U * chIndex);
            *minRate = 0U;
            *maxRate = 3U;
        }
        else if(chIndex < 72U){
            
            *freq = 903000000U + ( 200000U * (chIndex - 64U));
            *minRate = 4U;
            *maxRate = 3U;
        }
        else{
            
            retval = false;
        }
        break;
                    
    case CN_470_510:                    
    
        if(chIndex < 96U){
            
            *freq = 470300000U + ( 200000U * chIndex);
            *minRate = 0U;
            *maxRate = 5U;
            retval = true;
        }
        break;
    }
    
    return retval;
}

uint8_t Region_numChannels(enum lora_region region)
{
    uint8_t retval;
    
    switch(region){
    default:
    case EU_863_870:
    case EU_433:        
    case AS_923:
    case KR_920_923:
    case CN_779_787:     
    case IN_865_867:  
        retval = 16U;    
        break;                  
    case CN_470_510:
        retval = 96U;
        break;
    case US_902_928:            
    case AU_915_928:    
        retval = 72U;
        break;                
    }
    
    return retval;    
}

void Region_getDefaultChannels(enum lora_region region, void *receiver, void (*handler)(void *reciever, uint8_t chIndex, uint32_t freq, uint8_t minRate, uint8_t maxRate))
{
    LORA_PEDANTIC(handler != NULL)
    
    uint8_t minRate;
    uint8_t maxRate;
    
    switch(region){
    default:
    case EU_863_870:
    
        (void)upRateRange(region, 0U, &minRate, &maxRate);
        
        handler(receiver, 0U, 868100000U, minRate, maxRate);
        handler(receiver, 1U, 868300000U, minRate, maxRate);
        handler(receiver, 2U, 868500000U, minRate, maxRate);        
        break;
    
    case EU_433:
    
        (void)upRateRange(region, 0U, &minRate, &maxRate);
        
        handler(receiver, 0U, 433175000U, minRate, maxRate);
        handler(receiver, 1U, 433375000U, minRate, maxRate);
        handler(receiver, 2U, 433575000U, minRate, maxRate);        
        break;
        
    case AS_923:
    case KR_920_923:
    case CN_779_787:                      
    case US_902_928:            
    case AU_915_928:
    case IN_865_867:
        break;
    }
}

uint16_t Region_getOffTimeFactor(enum lora_region region, uint8_t band)
{
    uint16_t retval = 0U;
    
    switch(region){
    default:
    case EU_863_870:
    
        switch(band){
        case 0U:
        case 1U:
        case 4U:
            retval = 100U;  // 1.0%
            break;
        case 2U:
            retval = 1000U;  // 0.1%
            break;
        case 3U:        
            retval = 10U;  // 10.0%
            break;                    
        default:
            break;
        }
        break;
    
    case EU_433:        
        retval = 100U;
        break;    
    case AS_923:
    case KR_920_923:
    case CN_779_787:                      
    case US_902_928:            
    case AU_915_928:
    case IN_865_867:
        break;
    }
    
    return retval;    
}

bool Region_validateRate(enum lora_region region, uint8_t chIndex, uint8_t minRate, uint8_t maxRate)
{
    bool retval = false;
    uint8_t min;
    uint8_t max;

    if(upRateRange(region, chIndex, &min, &max)){
        
        if((minRate >= min) && (maxRate <= max)){
            
            retval = true;
        }
    }    
    
    return retval;
}

bool Region_validateFreq(enum lora_region region, uint8_t chIndex, uint32_t freq)
{
    return true;
}

bool Region_getRX1DataRate(enum lora_region region, uint8_t tx_rate, uint8_t rx1_offset, uint8_t *rx1_rate)
{
    LORA_PEDANTIC(rx1_rate != NULL)

    bool retval = false;
    
    const uint8_t *ptr = NULL;
    uint16_t index;
    size_t size;
    
    switch(region){
    default:
    case EU_863_870:
    case EU_433:
    {
        static const uint8_t rates[] PROGMEM = {
            0U, 0U, 0U, 0U, 0U, 0U,   // DR0 upstream
            1U, 0U, 0U, 0U, 0U, 0U,   // DR1 upstream
            2U, 1U, 0U, 0U, 0U, 0U,   // DR2 upstream
            3U, 2U, 1U, 0U, 0U, 0U,   // DR3 upstream
            4U, 3U, 2U, 1U, 0U, 0U,   // DR4 upstream
            5U, 4U, 3U, 2U, 1U, 0U,   // DR5 upstream
            6U, 5U, 4U, 3U, 2U, 1U,   // DR6 upstream
            7U, 6U, 5U, 4U, 3U, 2U,   // DR7 upstream 
        };

        index = (tx_rate * 6U) + rx1_offset;
        ptr = rates;
        size = sizeof(rates);
    }
        break;    
    case AS_923:
    case KR_920_923:
    case CN_779_787:
    case IN_865_867:  
        break;                      
    case US_902_928:            
    case AU_915_928:
    {
        static const uint8_t rates[] PROGMEM = {
            10U, 9U,  8U,  8U,      // DR0 upstream
            11U, 10U, 9U,  8U,      // DR1 upstream
            12U, 11U, 10U, 9U,      // DR2 upstream
            13U, 12U, 11U, 10U,     // DR3 upstream
            13U, 13U, 12U, 11U,     // DR4 upstream
        };
        
        index = (tx_rate * 4U) + rx1_offset;
        ptr = rates;
        size = sizeof(rates);
    }
        break;
    }
    
    if((ptr != NULL) && (index < size)){
        
        (void)memcpy_P(rx1_rate, &ptr[index], sizeof(*rx1_rate));
        retval = true;
    }
        
    return retval;    
}

bool Region_getRX1Freq(enum lora_region region, uint32_t txFreq, uint32_t *freq)
{
    bool retval = false;
    uint8_t chIndex;
    
    switch(region){
    default:
    case EU_863_870:
    case EU_433:
    case AS_923:
    case KR_920_923:
    case CN_779_787:
    case CN_470_510:    
    case IN_865_867:                  
        *freq = txFreq;
        break;                      
    case US_902_928:            
    case AU_915_928:    
        if(freqToChannel(region, txFreq, &chIndex)){
            
        }        
        break;                
    }
    
    return retval;
}

uint16_t Region_getMaxFCNTGap(enum lora_region region)
{
    uint16_t retval;
    
    switch(region){
    default:
    case EU_863_870:
    case EU_433:
    case AS_923:
    case KR_920_923:
    case CN_779_787:
    case CN_470_510:                      
    case US_902_928:            
    case AU_915_928:
    case IN_865_867:
        retval = 16384U;
        break;      
    }
    
    return retval;    
}

uint8_t Region_getRX1Delay(enum lora_region region)
{
    uint16_t retval;
    
    switch(region){
    default:
    case EU_863_870:
    case EU_433:
    case AS_923:
    case KR_920_923:
    case CN_779_787:
    case CN_470_510:                      
    case US_902_928:            
    case AU_915_928:
    case IN_865_867:
        retval = 1U;
        break;      
    }
    
    return retval; 
}

uint8_t Region_getJA1Delay(enum lora_region region)
{
    uint8_t retval;
    
    switch(region){
    default:
    case EU_863_870:
    case EU_433:
    case AS_923:
    case KR_920_923:
    case CN_779_787:
    case CN_470_510:                      
    case US_902_928:            
    case AU_915_928:
    case IN_865_867:
        retval = 5U;
        break;      
    }
    
    return retval;
}

uint8_t Region_getRX1Offset(enum lora_region region)
{
    uint8_t retval;
    
    switch(region){
    default:
    case EU_863_870:
    case EU_433:
    case AS_923:
    case KR_920_923:
    case CN_779_787:
    case CN_470_510:                      
    case US_902_928:            
    case AU_915_928:
    case IN_865_867:
        retval = 0U;
        break;      
    }
    
    return retval; 
}

uint32_t Region_getRX2Freq(enum lora_region region)
{
    uint32_t retval;
    
    switch(region){
    default:
    case EU_863_870:
        retval = 869525000U;
        break;    
    case EU_433:
        retval = 434665000U;
        break;
    case AS_923:
        retval = 923200000U;
        break;
    case KR_920_923:
        retval = 921900000U;
        break;
    case CN_779_787:
        retval = 786000000U;
        break;        
    case CN_470_510:                      
        retval = 505300000U;
        break;
    case US_902_928:            
    case AU_915_928:        
        retval = 923300000U;
        break;        
    case IN_865_867:
        retval = 866550000U;
        break;
    }
    
    return retval; 
}

uint8_t Region_getRX2Rate(enum lora_region region)
{
    uint8_t retval;
    
    switch(region){
    default:
    case EU_863_870:
    case EU_433:        
    case KR_920_923:        
    case CN_779_787:
    case CN_470_510:    
        retval = 0U;
        break;                          
    case AS_923:
    case IN_865_867:
        retval= 2U;
        break;
    case US_902_928:            
    case AU_915_928:
        retval = 8U;
        break;      
    }
    
    return retval; 
}

uint8_t Region_getADRAckLimit(enum lora_region region)
{
    uint8_t retval;
    
    switch(region){
    default:
    case EU_863_870:
    case EU_433:
    case AS_923:
    case KR_920_923:
    case CN_779_787:
    case CN_470_510:                      
    case US_902_928:            
    case AU_915_928:
    case IN_865_867:
        retval = 64U;
        break;      
    }
    
    return retval; 
}

uint8_t Region_getADRAckDelay(enum lora_region region)
{
    uint8_t retval;
    
    switch(region){
    default:
    case EU_863_870:
    case EU_433:
    case AS_923:
    case KR_920_923:
    case CN_779_787:
    case CN_470_510:                      
    case US_902_928:            
    case AU_915_928:
    case IN_865_867:
        retval = 32U;
        break;      
    }
    
    return retval; 
}

uint8_t Region_getADRAckTimeout(enum lora_region region)
{
    uint8_t retval;
    
    switch(region){
    default:
    case EU_863_870:
    case EU_433:
    case AS_923:
    case KR_920_923:
    case CN_779_787:
    case CN_470_510:                      
    case US_902_928:            
    case AU_915_928:
    case IN_865_867:
        retval = 2U;
        break;      
    }
    
    return retval; 
}

uint8_t Region_getADRAckDither(enum lora_region region)
{
    uint8_t retval;
    
    switch(region){
    default:
    case EU_863_870:
    case EU_433:
    case AS_923:
    case KR_920_923:
    case CN_779_787:
    case CN_470_510:                      
    case US_902_928:            
    case AU_915_928:
    case IN_865_867:
        retval = 1U;
        break;      
    }
    
    return retval; 
}

uint8_t Region_getTXRate(enum lora_region region)
{
    uint8_t retval;
    
    switch(region){
    default:
    case EU_863_870:
    case EU_433:
        retval = 5U;
        break;
    case AS_923:
    case KR_920_923:
    case CN_779_787:
    case CN_470_510:                      
    case US_902_928:            
    case AU_915_928:
    case IN_865_867:
        retval = 4U;
        break;      
    }
    
    return retval; 
}

uint8_t Region_getTXPower(enum lora_region region)
{
    uint8_t retval;
    
    switch(region){
    default:
    case EU_863_870:
    case EU_433:
    case AS_923:
    case KR_920_923:
    case CN_779_787:
    case CN_470_510:                      
    case US_902_928:            
    case AU_915_928:
    case IN_865_867:
        retval = 0U;
        break;      
    }
    
    return retval; 
}

/* static functions ***************************************************/

static bool upRateRange(enum lora_region region, uint8_t chIndex, uint8_t *minRate, uint8_t *maxRate)
{
    bool retval = false;
    
    switch(region){    
    case EU_863_870:
    case EU_433:
    
        if(chIndex < 16U){
    
            *minRate = 0U;
            *maxRate = 5U;
            retval = true;
        }
        break;
    
    case AS_923:
    case KR_920_923:
    case CN_779_787:
    case CN_470_510:                
        break;
    case US_902_928:            
    case AU_915_928:
    
        if(chIndex <= 71U){
    
            if(chIndex <= 63U){
                
                *minRate = 0U;
                *maxRate = 3U;
            }
            else{ 
                
                *minRate = 4U;
                *maxRate = 4U;                
            }            
            
            retval = true;
        }
        break;        
    
    default:
        break;      
    }
    
    return retval;
}

static bool freqToChannel(enum lora_region region, uint32_t freq, uint8_t *chIndex)
{
    bool retval = false;
    
    switch(region){
    default:
    case EU_863_870:
    case EU_433:
    case AS_923:
    case KR_920_923:
    case CN_779_787:  
        break;                      
    case US_902_928:            
    case AU_915_928:
        break;
    }
    
    return retval;    
}

static bool getRateAndPayload(enum lora_region region, uint8_t rate, enum lora_spreading_factor *sf, enum lora_signal_bandwidth *bw, uint8_t *payload)
{
    LORA_PEDANTIC(sf != NULL)
    LORA_PEDANTIC(bw != NULL)
    LORA_PEDANTIC(payload != NULL)
    
    bool retval = false;
    
    switch(region){
    case EU_863_870:
    case EU_433:
    
        retval = true;

        switch(rate){
        case 0U:
            *sf = SF_12;
            *bw = BW_125;
            *payload = 59U;
            break;
        case 1U:
            *sf = SF_11;
            *bw = BW_125;
            *payload = 59U;
            break;
        case 2U:
            *sf = SF_10;
            *bw = BW_125;
            *payload = 59U;
            break;
        case 3U:
            *sf = SF_9;
            *bw = BW_125;
            *payload = 123U;
            break;
        case 4U:
            *sf = SF_8;
            *bw = BW_125;
            *payload = 230U;
            break;
        case 5U:
            *sf = SF_7;
            *bw = BW_125;
            *payload = 230U;
            break;
        case 6U:
            *sf = SF_7;
            *bw = BW_250;
            *payload = 230U;
            break;                
        default:
            retval = false;
            break;
        }
        break;
        
    case US_902_928:            
    case AU_915_928:
    
        retval = true;
    
        switch(rate){
        case 0U:
            *sf = SF_10;
            *bw = BW_125;
            *payload = 19U;
            break;
        case 1U:
            *sf = SF_9;
            *bw = BW_125;
            *payload = 61U;
            break;
        case 2U:
            *sf = SF_8;
            *bw = BW_125;
            *payload = 133U;
            break;
        case 3U:
            *sf = SF_7;
            *bw = BW_125;
            *payload = 250U;
            break;
        case 4U:
        case 12U:
            *sf = SF_8;
            *bw = BW_500;
            *payload = 250U;
            break;
        case 8U:
            *sf = SF_12;
            *bw = BW_500;
            *payload = 61U;
            break;
        case 9U:
            *sf = SF_11;
            *bw = BW_500;
            *payload = 137U;
            break;
        case 10U:
            *sf = SF_10;
            *bw = BW_500;
            *payload = 250U;
            break;
        case 11U:
            *sf = SF_9;
            *bw = BW_500;
            *payload = 250U;
            break;        
        case 13U:
            *sf = SF_7;
            *bw = BW_500;
            *payload = 250U;
            break;
        default:
            retval = false;
            break;
        }    
        break;
        
    case AS_923:
    case KR_920_923:
    case CN_779_787:
    case CN_470_510:   
    case IN_865_867:
    default:
        break;
    }
    
    return retval;
}

#if !defined(LORA_AVR)
    #undef memcpy_P
#endif
