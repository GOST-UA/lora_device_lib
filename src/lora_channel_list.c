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

#include "lora_channel_list.h"
#include "lora_region.h"
#include "lora_debug.h"

#include <string.h>

/* static prototypes **************************************************/

static void cycleChannel(struct lora_channel_list *self);
static void addDefaultChannel(void *receiver, uint8_t chIndex, uint32_t freq);

/* functions **********************************************************/

void ChannelList_init(struct lora_channel_list *self, const struct lora_region *region)
{
    LORA_PEDANTIC(self != NULL)
    LORA_PEDANTIC(region != NULL)
    
    size_t i;
    
    (void)memset(self, 0, sizeof(*self));

    for(i=0; i < sizeof(self->bands)/sizeof(*self->bands); i++){

        self->bands[i].channel = -1;
    }

    self->region = region;
    
    Region_getDefaultChannels(region, self, addDefaultChannel);
    
    self->nextBand = -1;
    self->nextChannel = -1;
    self->numUnmasked = 0;
}

bool ChannelList_add(struct lora_channel_list *self, uint8_t chIndex, uint32_t freq)
{
    LORA_PEDANTIC(self != NULL)
    
    bool retval = false;
    struct lora_channel *channel = NULL;
    uint8_t band;

    if(chIndex < sizeof(self->channels)/sizeof(*self->channels)){
        
        if(freq == 0){
            
            ChannelList_remove(self, chIndex);
            retval = true;
        }
        else{
            
            channel = &self->channels[chIndex];

            if(Region_validateFrequency(self->region, freq, &band)){

                if(channel->freq != 0U){
                    
                    ChannelList_remove(self, chIndex);
                }

                channel->freq = freq;
                channel->band = (int)band;
                
                self->bands[channel->band].numUnmasked++;
                self->numUnmasked++;

                if(self->numUnmasked == 1){

                    cycleChannel(self);
                }

                retval = true;
            }
        }
    }
    
    return retval;
}

void ChannelList_remove(struct lora_channel_list *self, uint8_t chIndex)
{
    LORA_PEDANTIC(self != NULL)
    
    if(chIndex < sizeof(self->channels)/sizeof(*self->channels)){
        
        if(self->channels[chIndex].freq > 0){
    
            self->channels[chIndex].freq = 0U;        
            
            if(!self->channels[chIndex].masked){
                self->bands[self->channels[chIndex].band].numUnmasked--;
                self->numUnmasked--;
            }
            if(self->nextChannel == chIndex){ 
                cycleChannel(self); 
            }
        }
    }
}

bool ChannelList_constrainRate(struct lora_channel_list *self, uint8_t chIndex, uint8_t minRate, uint8_t maxRate)
{
    LORA_PEDANTIC(self != NULL)
    
    bool retval = false;
    
    if(chIndex < sizeof(self->channels)/sizeof(*self->channels)){
        
        if(self->channels[chIndex].freq > 0U){
            
            if(minRate <= maxRate){
                
                if(Region_validateRate(self->region, chIndex, minRate) && Region_validateRate(self->region, chIndex, maxRate)){

                    self->channels[chIndex].minRate = minRate;
                    self->channels[chIndex].maxRate = maxRate;
                        
                    retval = true;
                }
            }
        }        
    }
    
    return retval;
}

bool ChannelList_mask(struct lora_channel_list *self, uint8_t chIndex)
{
    LORA_PEDANTIC(self != NULL)
    
    bool retval = false;
    
    if(chIndex < sizeof(self->channels)/sizeof(*self->channels)){
        
        if(self->channels[chIndex].freq > 0U){
        
            if(!self->channels[chIndex].masked){

                self->channels[chIndex].masked = true;
                self->bands[self->channels[chIndex].band].numUnmasked--;
                self->numUnmasked--;

                if(self->nextChannel == (int)chIndex){ 
                    
                    cycleChannel(self); 
                }
            }
        
            retval = true;
        }       
    }
        
    return retval;
}

void ChannelList_unmask(struct lora_channel_list *self, uint8_t chIndex)
{
    LORA_PEDANTIC(self != NULL)
    
    if(chIndex < sizeof(self->channels)/sizeof(*self->channels)){
        
        self->channels[chIndex].masked = false;
        self->bands[self->channels[chIndex].band].numUnmasked++;
        self->numUnmasked++;

        if(self->numUnmasked == 1){ 
            
            cycleChannel(self); 
        }
    }
}

uint64_t ChannelList_waitTime(const struct lora_channel_list *self, uint64_t timeNow)
{
    LORA_PEDANTIC(self != NULL)
    
    uint32_t retval;
    
    if((self->nextBand == -1) || (timeNow >= self->bands[self->nextBand].timeReady)){

        retval = 0U;
    }
    else{

        retval = self->bands[self->nextBand].timeReady - timeNow;
    }

    return retval;
}

void ChannelList_registerTransmission(struct lora_channel_list *self, uint64_t time, uint32_t airTime)
{
    LORA_PEDANTIC(self != NULL)
    
    if(self->nextBand != -1){

        uint16_t offTimeFactor;
        Region_getOffTimeFactor(self->region, self->nextBand, &offTimeFactor);

        uint64_t offTime_us = (uint64_t)airTime * (uint64_t)offTimeFactor;

        self->bands[self->nextBand].timeReady = time + offTime_us;
        cycleChannel(self);
    }
}

size_t ChannelList_capacity(const struct lora_channel_list *self)
{
    LORA_PEDANTIC(self != NULL)
    
    return sizeof(self->channels)/sizeof(*self->channels);
}

bool ChannelList_txSetting(const struct lora_channel_list *self, struct lora_channel_setting *setting)
{
    LORA_PEDANTIC(self != NULL)
    
    bool retval = false;
    
    if(self->nextChannel != -1){
    
        setting->freq = self->channels[self->nextChannel].freq;
        setting->chIndex = self->nextChannel;
        setting->minRate = self->channels[self->nextChannel].minRate;
        setting->maxRate = self->channels[self->nextChannel].maxRate;
        setting->numRates = Region_getTXRates(self->region, setting->chIndex, &setting->rates); 
        
        retval = true;
    }
        
    return retval;
}

void ChannelList_rx2Setting(const struct lora_channel_list *self, uint32_t *freq, uint8_t *rate)
{
    LORA_PEDANTIC(self != NULL)
        
    const struct lora_region_default *defaults;
    
    defaults = Region_getDefaultSettings(self->region);
        
    LORA_PEDANTIC(defaults != NULL)
        
    *freq = defaults->rx2_freq;
    *rate = defaults->rx2_rate;        
}

const struct lora_region *ChannelList_region(const struct lora_channel_list *self)
{
    LORA_PEDANTIC(self != NULL)
    
    return self->region;
}

/* static functions ***************************************************/

static void cycleChannel(struct lora_channel_list *self)
{
    uint32_t nextTime = UINT32_MAX;
    bool atLeastOneChannel = false;
    int channelCount;
    int bandCount;
    const int numChannels = (int)(sizeof(self->channels)/sizeof(*self->channels));

    self->nextBand = -1;

    // find the next available band with an unmasked channel
    for(bandCount=0; bandCount < (int)(sizeof(self->bands)/sizeof(*self->bands)); bandCount++){

        if(self->bands[bandCount].numUnmasked > 0){

            if(self->bands[bandCount].timeReady < nextTime){

                nextTime = self->bands[bandCount].timeReady;
                atLeastOneChannel = true;
                self->nextBand = bandCount;
            }
        }
    }

    // choose the next channel on the next band
    if(atLeastOneChannel){

        self->nextChannel = (self->bands[self->nextBand].channel == -1) ? 0 : self->bands[self->nextBand].channel + 1;
        
        for(channelCount=0; channelCount < numChannels; channelCount++, self->nextChannel++){

            if(self->nextChannel >= numChannels){

                self->nextChannel = 0;
            }

            if((self->channels[self->nextChannel].freq != 0U) && (self->channels[self->nextChannel].band == self->nextBand) && !self->channels[self->nextChannel].masked){

                self->bands[self->nextBand].channel = self->nextChannel;
                break;
            }
        }

        // if there is a band, there must be a channel
        LORA_ASSERT(channelCount != numChannels)

        // failsafe if asserts are disabled
        if(channelCount == numChannels){

            self->bands[self->nextBand].channel = -1;
            self->nextChannel = -1;
            self->nextBand = -1;
        }
    }
    else{

        self->nextBand = -1;
        self->nextChannel = -1;
    }
}

static void addDefaultChannel(void *receiver, uint8_t chIndex, uint32_t freq)
{
    struct lora_channel_list *self = (struct lora_channel_list *)receiver;    
    if(!ChannelList_add(self, chIndex, freq)){
        
        LORA_ERROR("could not add default channel")
        LORA_ASSERT(false)
    }
}
