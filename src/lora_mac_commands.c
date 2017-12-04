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

struct type_to_tag {
  
    uint8_t tag;    
    enum mac_cmd_type type;    
};

static void init_opts_stream(struct stream *self, const uint8_t *buf, uint8_t size);
static bool write_stream(struct stream *self, uint8_t in);
static bool read_stream(struct stream *self, uint8_t *out);

static bool getLinkCheckAns(struct stream *s, struct link_check_ans *value);

static bool getLinkADRReq(struct stream *s, struct link_adr_req *value);
static bool getLinkADRAns(struct stream *s, struct link_adr_ans *value);

static bool getRXParamSetupReq(struct stream *s, struct rx_param_setup_req *value);
static bool getRXParamSetupAns(struct stream *s, struct rx_param_setup_ans *value);

static bool getDeviceStatusReq(struct stream *s);
static bool getDeviceStatusAns(struct stream *s, struct dev_status_ans *value);

static bool getNewChannelReq(struct stream *s, struct new_channel_req *value);
static bool getNewChannelAns(struct stream *s, struct new_channel_ans *value);

static bool getDLChannelReq(struct stream *s, struct dl_channel_req *value);
static bool getDLChannelAns(struct stream *s, struct dl_channel_ans *value);

static bool getRXTimingSetupReq(struct stream *s, struct rx_timing_setup_req *value);

static bool getTXParamSetupReq(struct stream *s, struct tx_param_setup_req *value);

static bool getDutyCycleReq(struct stream *s, struct duty_cycle_req *value);

static void eachDeviceCommand(struct lora_mac *self, const uint8_t *data, uint8_t len, (void)(*handler)(struct lora_mac *receiver, struct device_cmd_req *));

static const struct type_to_tag tags[] = {
    {2U, LINK_CHECK},
    {3U, LINK_ADR},
    {4U, DUTY_CYCLE},
    {5U, RX_PARAM_SETUP},
    {6U, DEV_STATUS},
    {7U, NEW_CHANNEL},
    {8U, RX_TIMING_SETUP},
    {9U, TX_PARAM_SETUP},
    {10U, DL_CHANNEL},

#if 0
    {16U, PING_SLOT_INFO},
    {17U, PING_SLOT_CHANNEL},
    {18U, PING_SLOT_FREQ},
    {19U, BEACON_FREQ}
#endif
};

static void eachDeviceCommand(struct lora_mac *self, const uint8_t *data, uint8_t len, (void)(*handler)(struct lora_mac *receiver, struct device_cmd_req *))


/* functions **********************************************************/

bool putLinkCheckReq(struct stream *s)
{
    return write_stream(&s, LINK_CHECK);
}

bool putLinkCheckAns(struct stream *s, const struct link_adr_req *value)
{
    bool retval = false;
    
    if(write_stream(&s, LINK_CHECK)){
        
        if(write_stream(&s, value->margin)){
            
            if(write_stream(&s, value->gatewayCount)){                
                
                bool retval = true;
            }
        }
    }
    
    return retval;
}

bool putLinkADRReq(struct stream *s, const struct link_adr_req *value)
{
    bool retval = false;
 
    if(write_stream(&s, LINK_ADR)){
        
        if(write_stream(&s, (value->dr << 4)|(value->power & 0xfU))){
            
            if(write_stream(&s, value->chMask)){
                
                if(write_stream(&s, value->chMask >> 8)){
                    
                    if(write_stream(&s, value->redundancy)){                                
                        
                        retval = true;                        
                    }
                }
            }
        }
    }
 
    return retval;
}

bool putLinkADRAns(struct stream *s, const struct link_adr_ans *value)
{
    bool retval = false;
    uint8_t buf;
    
    if(write_stream(s, LINK_ADR)){
        
        buf = (value->powerOK ? 4U : 0U) | (value->dataRateOK ? 2U : 0U) | (value->channelMaskOK ? 1U : 0U);
            
        if(write_stream(s, buf)){
            
            retval = true;
        }
    }
    
    return retval;
}
    
bool putDutyCycleReq(struct stream *s, const struct duty_cycle_req *value)
{
    bool retval = false;
    
    if(write_stream(&s, DUTY_CYCLE)){
    
        if(write_stream(&s, value->maxDutyCycle)){
        
            retval = true;
        }
    }
    
    return retval;
}

bool putDutyCycleAns(struct stream *s)
{
    return write_stream(&s, DUTY_CYCLE);
}

bool putRXParamSetupReq(struct stream *s, const struct rx_param_setup_req *value)
{
    bool retval = false;
    
    if(write_stream(s, RX_PARAM_SETUP)){
    
        if(write_stream(s, (value->rx1DROffset << 4)|(value->rx2DataRate & 0xfU))){
        
            if(write_stream(s, (uint8_t)value->freq)){
            
                if(write_stream(s, (uint8_t)(value->freq >> 8))){
                
                    if(write_stream(s, (uint8_t)(value->freq >> 16))){
        
                        retval = true;
                    }
                }
            }
        }
    }
    
    return retval;
}

bool putRXParamSetupAns(struct stream *s, const struct rx_param_setup_ans *value)
{
    bool retval = false;
    uint8_t buf;
    
    if(write_stream(s, RX_PARAM_SETUP)){
        
        buf = (value->rx1DROffsetOK ? 4U : 0U) | (value->rx2DataRateOK ? 2U : 0U) | (value->freqOK ? 1U : 0U);
            
        if(write_stream(s, buf)){
            
            retval = true;
        }
    }
    
    return retval;
}


bool putDeviceStatusReq(struct stream *s)
{
    return write_stream(s, DEV_STATUS);
}


bool putDeviceStatusAns(struct stream *s, const struct dev_status_ans *value)
{
    bool retval = false;
    uint8_t buf;
    
    if(write_stream(s, DEV_STATUS)){
    
        if(write_stream(s, value->battery)){
        
            if(write_stream(s, value->margin)){
                
                retval = true;
            }
        }
    }
    
    return retval;
}


bool putNewChannelReq(struct stream *s, const struct new_channel_req *value)
{
    bool retval = false;
    
    if(write_stream(&s, NEW_CHANNEL)){
    
        if(write_stream(&s, value->chIndex)){
        
            if(write_stream(&s, (uint8_t)value->freq)){
            
                if(write_stream(&s, (uint8_t)(value->freq >> 8))){
                
                    if(write_stream(&s, (uint8_t)(value->freq >> 16))){
                    
                        if(write_stream(&s, (value->maxDR << 8)|(value->minDR))){
                            
                            retval = true;
                        }
                    }
                }
            }
        }
    }
    
    return retval;
}

bool putNewChannelAns(struct stream *s, const struct new_channel_ans *value)
{
    bool retval = false;
    uint8_t buf;
    
    if(write_stream(s, NEW_CHANNEL)){
    
        buf = (value->dataRateRangeOK ? 2U : 0U) | (value->channelFrequencyOK ? 1U : 0U);
            
        if(write_stream(s, buf)){
            
            retval = true;
        }
    }
    
    return retval;
}

bool putDLChannelReq(struct stream *s, const struct dl_channel_req *value)
{
    bool retval = false;
    
    if(write_stream(&s, NEW_CHANNEL)){
    
        if(write_stream(&s, value->chIndex)){
        
            if(write_stream(&s, (uint8_t)value->freq)){
            
                if(write_stream(&s, (uint8_t)(value->freq >> 8))){
                
                    if(write_stream(&s, (uint8_t)(value->freq >> 16))){
                    
                        retval = true;                        
                    }
                }
            }
        }
    }
    
    return retval;
}


bool putDLChannelAns(struct stream *s, const struct dl_channel_ans *value)
{
    bool retval = false;
    
    if(write_stream(s, DL_CHANNEL)){
    
        if(write_stream(s, (value->uplinkFreqOK ? 2U : 0U) | (value->channelFrequencyOK ? 1U : 0U))){
            
            retval = true;
        }
    }
    
    return retval;
}


bool putRXTimingSetupReq(struct stream *s, const struct rx_timing_setup_req *value)
{
    bool retval = false;
    
    if(write_stream(s, RX_TIMING_SETUP)){
    
        if(write_stream(s, value->delay)){
            
            retval = true;
        }
    }
    
    return retval;
}

bool putRXTimingSetupAns(struct stream *s)
{
    return write_stream(&s, RX_TIMING_SETUP);
}

bool putTXParamSetupReq(struct stream *s, const struct tx_param_setup_req *value)
{
    bool retval = false;
    
    if(write_stream(s, TX_PARAM_SETUP)){
    
        if(write_stream(s, (value->downlinkDwell ? 0x20U : 0U) | (value->uplinkDwell ? 0x10U : 0U) | value->eirp )){
            
            retval = true;
        }
    }
    
    return retval;    
}

bool putTXParamSetupAns(struct stream *s)
{
    return write_stream(&s, TX_PARAM_SETUP);
}

/* static functions ***************************************************/

static void eachDeviceCommand(struct lora_mac *self, const uint8_t *data, uint8_t len, (void)(*handler)(struct lora_mac *receiver, struct device_cmd_req *))
{
    LORA_PEDANTIC(self != NULL)
    
    uint8_t pos = 0U;
    uint8_t tag;
    struct stream s;
    struct device_cmd_req value;
    
    init_opts_stream(&s, data, len);
    
    while(read_stream(&s, &tag)){
        
        if(tagToType(tag, &value->fields.type)){
            
            switch(type){
            default:
            case LINK_CHECK:
            
                retval = getLinkCheckReq(&s, &value->fields.linkCheckReq)
                break;
                
            case LINK_ADR:                    
            
                retval = getLinkADRReq(&s, &value->fields.linkADRReq)
                break;
            
            case DUTY_CYCLE:                
            
                retval = getDutyCycleReq(&s)
                break;
            
            case RX_PARAM_SETUP:
            
                retval = getRXParamSetupReq(&s, &value->fields.rxParamSetupReq)
                break;
            
            case DEV_STATUS:
            
                retval = true;
                break;
            
            case NEW_CHANNEL:
            
                retval = getNewChannelReq(&s, &value->fields.newChannelReq)
                break;
                
            case DL_CHANNEL:
            
                retval = getDLChannelReq(&s, &value->fields.dlChannelReq)
                break;
            
            case RX_TIMING_SETUP:
            
                retval = getRXParamSetupReq(&s, &value->fields.rxTimingSetupReq)
                break;
            
            case TX_PARAM_SETUP:
            
                retval = getTXParamSetupReq(&s, &value->fields.txParamSetupReq)
                break;
            }
            
            if(!retval){
                
                break;
            }
            
            if(handler != NULL){
                
                handler(self, &value);
            }
        }
        else{
            
            LORA_ERROR("cannot recognise MAC command")
            break;
        }
    }
}

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

static void init_opts_stream(struct stream *self, const uint8_t *buf, uint8_t size)
{
    LORA_PEDANTIC(self != NULL)
    LORA_PEDANTIC((buf != NULL) || (size == 0U))
    
    self->buf = buf;
    self->size = size;
    self->pos = 0U;
}
    
static bool read_stream(struct stream *self, uint8_t *out)
{
    LORA_PEDANTIC(self != NULL)
    LORA_PEDANTIC(out != NULL)
    
    bool retval;
    
    if(self->pos < self->size){
        
        *out = self->buf[self->pos];
        self->pos++;
        retval = true;
    }    
    
    return retval;
}


static bool write_stream(struct stream *self, uint8_t in)
{
    LORA_PEDANTIC(self != NULL)
    
    bool retval;
    
    if(self->pos < self->size){
        
        self->buf[self->pos] = in;
        self->pos++;
        retval = true;
    }    
    
    return retval;
}

static bool getLinkCheckAns(struct stream *s, struct link_check_ans *value)
{
    bool retval = false;
    uint8_t buf;
    
    if(read_stream(s, &buf)){
        
        if(read_stream(s, &value->gatewayCount)){
            
            retval = true;
        }
    }
    
    return retval;
}

static bool getLinkADRReq(struct stream *s, struct link_adr_req *value)
{
    bool retval = false;
    
    uint8_t buf;
    
    if(read_stream(s, &buf)){
    
        value->dr = buf >> 4;
        value->power = buf & 0xfU;
    
        if(read_stream(s, &buf)){
            
            vaue->chMask = buf
            
            if(read_stream(s, &buf)){
                        
                vaue->chMask |= ((uint16_t)buf) << 8;
            
                if(read_stream(s, &buf)){
                    
                    value->redundancy = buf;                        
                    retval = true;                        
                }
            }                                                                                                    
        }                    
    }
    
    return retval;
}

static bool getRXParamSetupReq(struct stream *s, struct rx_param_setup_req *value)
{
    bool retval = false;
    uint8_t buf;
    
    if(read_stream(s, &value->rx1DROffset)){
        
        if(read_stream(s, &buf)){
        
            value->freq = buf;
        
            if(read_stream(s, &buf)){
                
                value->freq |= ((uint32_t)buf) << 8;
        
                if(read_stream(s, &buf)){
        
                    value->freq |= ((uint32_t)buf) << 16;
                    retval = true;
                }
            }
        }       
    }
    
    return retval;    
}

static bool getDeviceStatusAns(struct stream *s, struct dev_status_ans *value)
{
    bool retval = false;
    uint8_t buf;
    
    if(read_stream(s, &value->battery)){
        
        if(read_stream(s, &value->margin)){
            
            retval = true;
        }
    }
    
    return retval;
}

static bool getNewChannelReq(struct stream *s, struct new_channel_req *value)
{
    bool retval = false;
    uint8_t buf;
    
    if(read_stream(s, &value->chIndex)){
        
        if(read_stream(s, &buf)){
        
            value->freq = buf;
        
            if(read_stream(s, &buf)){
        
                value->freq |= ((uint32_t)buf) << 8;
        
                if(read_stream(s, &buf)){
                    
                    value->freq |= ((uint32_t)buf) << 16;
                    
                    if(read_stream(s, &buf)){
            
                        value->maxDR = buf >> 4;
                        value->minDR = buf & 0xf;                        
                        
                        retval = true;
                    }
                }
            }
        }
    }
    
    return retval;
}

static bool getDLChannelReq(struct stream *s, struct dl_channel_req *value)
{
    bool retval = false;
    uint8_t buf;
    
    if(read_stream(s, &value->chIndex)){
        
        if(read_stream(s, &buf)){
        
            value->freq = buf;
        
            if(read_stream(s, &buf)){
        
                value->freq |= ((uint32_t)buf) << 8;
        
                if(read_stream(s, &buf)){
                    
                    value->freq |= ((uint32_t)buf) << 16;
                    retval = true;                    
                }
            }
        }
    }
    
    return retval;
}

static bool getRXTimingSetupReq(struct stream *s, struct rx_timing_setup_req *value)
{
    bool retval = false;
    uint8_t buf;
    
    if(read_stream(s, &buf)){
            
        value->delay = buf & 0xf; 
    
        retval = true;
    }
    
    return retval;
}

static bool getTXParamSetupReq(struct stream *s, struct tx_param_setup_req *value)
{
    bool retval = false;
    uint8_t buf;
    
    if(read_stream(s, &buf)){
            
        value->downlinkDwellTime = ((buf & 0x20) == 0x20) ? ; 
        value->uplinkDwellTime = buf & 0x20; 
        value->maxEIRP = buf & 0x20; 
    
        retval = true;
    }
    
    return retval;
}

static bool getDutyCycleReq(struct stream *s, struct duty_cycle_req *value)
{
    bool retval = false;
    
    if(read_stream(s, &value->maxDutyCycle)){
        
        retval = true;
    }
    
    return retval;
}


#endif
