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

#if 0

#include "lora_mac.h"

    struct lora_link_check_ans {
        
        uint32_t time;
        uint8_t gateway_count;
        uint8_t margin;                
    
    } lastLinkCheck;
    
    struct {
        
        bool linkCheck_pending;
        
        bool linkADR_pending;
        uint8_t linkADR[1];
        
        bool dutyCycle_pending;
        
        bool rxParamSetup_pending;
        uint8_t rxParamSetup[1];
        
        bool devStatus_pending;
        uint8_t devStatus[2];
        
        bool newChannel_pending;
        uint8_t newChannel[1];
        
        bool rxTimingSetup_pending;
        
        bool txParamSetup_pending;
        
        bool dlChannel_pending;
        uint8_t dlChannel[1]; 
        
    } cmd;

enum mac_cmd_type {
    LINK_CHECK,
    LINK_ADR,
    DUTY_CYCLE,
    RX_PARAM_SETUP,
    DEV_STATUS,
    NEW_CHANNEL,
    RX_TIMING_SETUP,
    TX_PARAM_SETUP,
    DL_CHANNEL,
    NUM_MAC_CMD
};

struct type_to_tag {
  
    uint8_t tag;
    uint8_t cmdPayloadSize;
    enum mac_cmd_type type;    
};

static const struct type_to_tag tags[] = {
    {2U, 2U, LINK_CHECK},
    {3U, 4U, LINK_ADR},
    {4U, 1U, DUTY_CYCLE},
    {5U, 4U, RX_PARAM_SETUP},
    {6U, 0U, DEV_STATUS},
    {7U, 5U, NEW_CHANNEL},
    {8U, 1U, RX_TIMING_SETUP},
    {9U, 1U, TX_PARAM_SETUP},
    {10U, 4U, DL_CHANNEL}
};

static uint8_t typeToTag(enum mac_cmd_type type)
{
    return tags[type].tag;
}
    
static bool tagToType(uint8_t *tag, enum mac_cmd_type *type)
{
    bool retval = false;
    size_t i;
    
    for(i=0U; i < sizeof(tags)/sizeof(*tags); i++){
        
        if(tags[i].tag == tag){
            
            *type = tags[i].type;
            retval = true;
            break;
        }
    }
    
    return retval;
}

void LoraMAC_processCommands(struct lora_mac *self, const uint8_t *data, uint8_t len)
{
    uint8_t pos = 0U;
    enum mac_cmd_type type;

    while(pos < len){

        if(tagToType(data[pos], &type)){
                
            pos++;
            
            if((len-pos) >= tags[type].cmdPayloadSize){
            
                switch(type)
                case LINK_CHECK:
                {
                    uint8_t margin;
                    uint8_t gatewayCount;

                    margin = data[pos];
                    gatewayCount = data[pos+1U];

                    self->lastLinkCheck.margin = margin;
                    self->lastLinkCheck.gatewayCount = gatewayCount;
                    self->lastLinkCheck.time = getTime();
                }
                    break;
                    
                case LINK_ADR:
                {                
                    uint8_t rate;
                    uint8_t power;
                    uint16_t mask;
                    uint8_t maskControl;
                    
                    rate = (data[pos] >> 4) & 0xfU; 
                    power = data[pos] & 0xfU
                    mask = data[pos+1U];
                    mask <<= 8;
                    mask |= data[pos+2U]
                    self->nbTrans = data[pos + 3U] & 0xf;
                    maskControl = (data[pos+3U] >> 4) & 0x7U;
                    
                    struct lora_adr_ans result = ChannelList_adrRequest(self->channels, rate, power, mask, maskControl);
                    
                    *self->cmd.linkADR = (result.powerOK ? 0x4U : 0x0U) |
                        (result.rateOK ? 0x2U : 0x0U) |
                        (result.maskOK ? 0x1U : 0x0U);
                    
                    self->cmd.linkADR_pending = true;
                }
                    break;

                case DUTY_CYCLE
                {
                    uint8_t duty;
                    
                    duty = in[pos];
                    
                    self->cmd.dutyCycle_pending = true;                    
                    break;
                    
                case RX_PARAM_SETUP
                {
                    uint8_t rx1DROffset;
                    uint8_t rx2DataRate;
                    uint32_t frequency;
                    
                    rx1DROffset = ((in[pos] >> 4) & 0x7U);
                    rx2DataRate = (in[pos] & 0xfU);
                    frquency = in[pos + 1U];
                    frequency <<= 8;
                    frquency |= in[pos + 2U];
                    frequency <<= 8;
                    frquency |= in[pos + 3U];
                    
                    *self->cmd.rxParamSetup = 0U;
                    self->cmd.rxParamSetup_pending = true;
                }
                    break;
                    
                case DEV_STATUS:
                
                    self->cmd.devStatus[0] = 0xffU;
                    self->cmd.devStatus[1] = 0x3fU;
                    
                    self->cmd.devStatus_pending = true;                    
                    break;
                    
                case NEW_CHANNEL
                {
                    uint8_t chIndex;
                    uint8_t freq;
                    uint8_t maxRate;
                    uint8_t minRate;
                    
                    chIndex = in[pos];
                    freq = in[pos + 1U];
                    freq <<= 8;
                    freq = in[pos + 2U];
                    freq <<= 8;
                    freq = in[pos + 3U];
                    maxRate = (in[pos + 4U] >> 4) & 0xfU;
                    minRate = in[pos + 4U] & 0xfU;
                    
                    *self->cmd.newChannel = 0U;
                    self->cmd.newChannel_pending = true;                    
                }
                    break;            
                    
                case RX_TIMING_SETUP
                {
                    uint8_t delay;
                    
                    delay = in[pos];
                    
                    self->cmd.rxTimingSetup_pending = true;                    
                }
                    break;
                case TX_PARAM_SETUP
                {
                    uint8_t dwellTime;
                    
                    dwellTime = in[pos];
                    
                    *self->cmd.newChannel = 0U;
                    self->cmd.newChannel_pending = true;                    
                }
                    break;
                case DL_CHANNEL
                {
                    uint8_t chIndex;
                    uint32_t freq;
                    
                    chIndex = in[pos];
                    freq = in[pos + 1U];
                    freq <<= 8;
                    freq = in[pos + 2U];
                    freq <<= 8;
                    freq = in[pos + 3U];
                    
                    *self->cmd.dlChannel = 0U;
                    self->cmd.dlChannel_pending = true;                    
                }
                    break;
                
                default:
                    break;
                }
                
                pos += cmdPayloadSize[type];
            }
            else{
                
                LORA_ERROR("MAC command shorter than expected")
                break;
            }
        }
        else{

            LORA_ERROR("cannot recognise MAC command")
            break;
        }
    }    
}

#endif
