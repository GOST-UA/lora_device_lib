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

#include "lora_mac_commands.h"
#include "lora_stream.h"
#include "lora_debug.h"

struct type_to_tag {
  
    uint8_t tag;    
    enum lora_mac_cmd_type type;    
};

static uint8_t typeToTag(enum lora_mac_cmd_type type);    
static bool tagToType(uint8_t tag, enum lora_mac_cmd_type *type);

static bool putU24(struct lora_stream *self, uint32_t in);
static bool putU16(struct lora_stream *self, uint16_t in);
static bool putU8(struct lora_stream *self, uint8_t in);

static bool getU8(struct lora_stream *self, uint8_t *out);
static bool getU16(struct lora_stream *self, uint16_t *out);
static bool getU24(struct lora_stream *self, uint32_t *out);

static bool getLinkCheckAns(struct lora_stream *s, struct lora_link_check_ans *value);

static bool getLinkADRReq(struct lora_stream *s, struct lora_link_adr_req *value);
static bool getLinkADRAns(struct lora_stream *s, struct lora_link_adr_ans *value);

static bool getRXParamSetupReq(struct lora_stream *s, struct lora_rx_param_setup_req *value);
static bool getRXParamSetupAns(struct lora_stream *s, struct lora_rx_param_setup_ans *value);

static bool getDevStatusAns(struct lora_stream *s, struct lora_dev_status_ans *value);

static bool getNewChannelReq(struct lora_stream *s, struct lora_new_channel_req *value);
static bool getNewChannelAns(struct lora_stream *s, struct lora_new_channel_ans *value);

static bool getDLChannelReq(struct lora_stream *s, struct lora_dl_channel_req *value);
static bool getDLChannelAns(struct lora_stream *s, struct lora_dl_channel_ans *value);

static bool getRXTimingSetupReq(struct lora_stream *s, struct lora_rx_timing_setup_req *value);

static bool getTXParamSetupReq(struct lora_stream *s, struct lora_tx_param_setup_req *value);

static bool getDutyCycleReq(struct lora_stream *s, struct lora_duty_cycle_req *value);

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
    {16U, PING_SLOT_INFO},
    {17U, PING_SLOT_CHANNEL},
    {18U, BEACON_TIMING},
    {19U, BEACON_FREQ}
};

/* functions **********************************************************/

bool MAC_putLinkCheckReq(struct lora_stream *s)
{
    return putU8(s, typeToTag(LINK_CHECK));
}

bool MAC_putLinkCheckAns(struct lora_stream *s, const struct lora_link_check_ans *value)
{
    bool retval = false;
    
    if(putU8(s, typeToTag(LINK_CHECK))){
        
        if(putU8(s, value->margin)){
            
            if(putU8(s, value->gwCount)){                
                
                retval = true;
            }
        }
    }
    
    return retval;
}

bool MAC_putLinkADRReq(struct lora_stream *s, const struct lora_link_adr_req *value)
{
    bool retval = false;
 
    if(putU8(s, typeToTag(LINK_ADR))){
        
        if(putU8(s, (value->dataRate << 4)|(value->txPower & 0xfU))){
            
            if(putU16(s, value->channelMask)){
                    
                if(putU8(s, value->redundancy)){                                
                    
                    retval = true;                        
                }                
            }
        }
    }
 
    return retval;
}

bool MAC_putLinkADRAns(struct lora_stream *s, const struct lora_link_adr_ans *value)
{
    bool retval = false;
    uint8_t buf;
    
    if(putU8(s, typeToTag(LINK_ADR))){
        
        buf = (value->powerOK ? 4U : 0U) | (value->dataRateOK ? 2U : 0U) | (value->channelMaskOK ? 1U : 0U);
            
        if(putU8(s, buf)){
            
            retval = true;
        }
    }
    
    return retval;
}
    
bool MAC_putDutyCycleReq(struct lora_stream *s, const struct lora_duty_cycle_req *value)
{
    bool retval = false;
    
    if(putU8(s, typeToTag(DUTY_CYCLE))){
    
        if(putU8(s, value->maxDutyCycle)){
        
            retval = true;
        }
    }
    
    return retval;
}

bool MAC_putDutyCycleAns(struct lora_stream *s)
{
    return putU8(s, typeToTag(DUTY_CYCLE));
}

bool MAC_putRXParamSetupReq(struct lora_stream *s, const struct lora_rx_param_setup_req *value)
{
    bool retval = false;
    
    if(putU8(s, typeToTag(RX_PARAM_SETUP))){
    
        if(putU8(s, (value->rx1DROffset << 4)|(value->rx2DataRate & 0xfU))){
        
            if(putU24(s, value->freq)){
                            
                retval = true;                    
            }
        }
    }
    
    return retval;
}

bool MAC_putRXParamSetupAns(struct lora_stream *s, const struct lora_rx_param_setup_ans *value)
{
    bool retval = false;
    
    if(putU8(s, typeToTag(RX_PARAM_SETUP))){
            
        if(putU8(s, (value->rx1DROffsetOK ? 4U : 0U) | (value->rx2DataRateOK ? 2U : 0U) | (value->channelOK ? 1U : 0U))){
            
            retval = true;
        }
    }
    
    return retval;
}


bool MAC_putDevStatusReq(struct lora_stream *s)
{
    return putU8(s, typeToTag(DEV_STATUS));
}


bool MAC_putDevStatusAns(struct lora_stream *s, const struct lora_dev_status_ans *value)
{
    bool retval = false;
    
    if(putU8(s, typeToTag(DEV_STATUS))){
    
        if(putU8(s, value->battery)){
        
            if(putU8(s, value->margin)){
                
                retval = true;
            }
        }
    }
    
    return retval;
}

bool MAC_putNewChannelReq(struct lora_stream *s, const struct lora_new_channel_req *value)
{
    bool retval = false;
    
    if(putU8(s, typeToTag(NEW_CHANNEL))){
    
        if(putU8(s, value->chIndex)){
        
            if(putU24(s, value->freq)){
                
                if(putU8(s, (value->maxDR << 4)|(value->minDR))){
                    
                    retval = true;
                }                    
            }
        }
    }
    
    return retval;
}

bool MAC_putNewChannelAns(struct lora_stream *s, const struct lora_new_channel_ans *value)
{
    bool retval = false;
    
    if(putU8(s, typeToTag(NEW_CHANNEL))){
    
        if(putU8(s, (value->dataRateRangeOK ? 2U : 0U) | (value->channelFrequencyOK ? 1U : 0U))){
            
            retval = true;
        }
    }
    
    return retval;
}

bool MAC_putDLChannelReq(struct lora_stream *s, const struct lora_dl_channel_req *value)
{
    bool retval = false;
    
    if(putU8(s, typeToTag(NEW_CHANNEL))){
    
        if(putU8(s, value->chIndex)){
        
            if(putU24(s, value->freq)){
                                
                retval = true;                        
            }
        }
    }
    
    return retval;
}

bool MAC_putDLChannelAns(struct lora_stream *s, const struct lora_dl_channel_ans *value)
{
    bool retval = false;
    
    if(putU8(s, typeToTag(DL_CHANNEL))){
    
        if(putU8(s, (value->uplinkFreqOK ? 2U : 0U) | (value->channelFrequencyOK ? 1U : 0U))){
            
            retval = true;
        }
    }
    
    return retval;
}

bool MAC_putRXTimingSetupReq(struct lora_stream *s, const struct lora_rx_timing_setup_req *value)
{
    bool retval = false;
    
    if(putU8(s, typeToTag(RX_TIMING_SETUP))){
    
        if(putU8(s, value->delay)){
            
            retval = true;
        }
    }
    
    return retval;
}

bool MAC_putRXTimingSetupAns(struct lora_stream *s)
{
    return putU8(s, typeToTag(RX_TIMING_SETUP));
}

bool MAC_putTXParamSetupReq(struct lora_stream *s, const struct lora_tx_param_setup_req *value)
{
    bool retval = false;
    
    if(putU8(s, typeToTag(TX_PARAM_SETUP))){
    
        if(putU8(s, (value->downlinkDwell ? 0x20U : 0U) | (value->uplinkDwell ? 0x10U : 0U) | value->maxEIRP )){
            
            retval = true;
        }
    }
    
    return retval;    
}

bool MAC_putTXParamSetupAns(struct lora_stream *s)
{
    return putU8(s, typeToTag(TX_PARAM_SETUP));
}

bool MAC_eachDownstreamCommand(void *receiver, const uint8_t *data, uint8_t len, void (*handler)(void *, const struct lora_downstream_cmd *))
{
    uint8_t tag;
    struct lora_stream s;
    struct lora_downstream_cmd value;
    bool retval = true;
    
    (void)Stream_initReadOnly(&s, data, len);
    
    while(retval){
        
        if(getU8(&s, &tag)){
        
            if(tagToType(tag, &value.type)){
                
                switch(value.type){
                default:
                case LINK_CHECK:
                
                    retval = getLinkCheckAns(&s, &value.fields.linkCheckAns);
                    break;
                    
                case LINK_ADR:                    
                
                    retval = getLinkADRReq(&s, &value.fields.linkADRReq);
                    break;
                
                case DUTY_CYCLE:                
                
                    retval = getDutyCycleReq(&s, &value.fields.dutyCycleReq);
                    break;
                
                case RX_PARAM_SETUP:
                
                    retval = getRXParamSetupReq(&s, &value.fields.rxParamSetupReq);
                    break;
                
                case DEV_STATUS:
                
                    retval = true;
                    break;
                
                case NEW_CHANNEL:
                
                    retval = getNewChannelReq(&s, &value.fields.newChannelReq);
                    break;
                    
                case DL_CHANNEL:
                
                    retval = getDLChannelReq(&s, &value.fields.dlChannelReq);
                    break;
                
                case RX_TIMING_SETUP:
                
                    retval = getRXTimingSetupReq(&s, &value.fields.rxTimingSetupReq);
                    break;
                
                case TX_PARAM_SETUP:
                
                    retval = getTXParamSetupReq(&s, &value.fields.txParamSetupReq);
                    break;
                }
                
                if(retval && (handler != NULL)){
                    
                    handler(receiver, &value);
                }
            }
            else{
                
                LORA_ERROR("cannot recognise MAC command")
                retval = false;
            }        
        }
        else{
            
            break;
        }
    }
    
    return retval;
}

bool MAC_eachUpstreamCommand(void *receiver, const uint8_t *data, uint8_t len, void (*handler)(void *, const struct lora_upstream_cmd *))
{
    uint8_t tag;
    struct lora_stream s;
    struct lora_upstream_cmd value;
    bool retval = true;
    
    (void)Stream_initReadOnly(&s, data, len);
    
    while(retval){
    
        if(getU8(&s, &tag)){
        
            if(tagToType(tag, &value.type)){
                
                switch(value.type){
                default:
                case LINK_CHECK:
                case DUTY_CYCLE:                
                case RX_TIMING_SETUP:
                case TX_PARAM_SETUP:
                
                    retval = true;
                    break;
                    
                case LINK_ADR:                    
                
                    retval = getLinkADRAns(&s, &value.fields.linkADRAns);
                    break;
                
                case RX_PARAM_SETUP:
                
                    retval = getRXParamSetupAns(&s, &value.fields.rxParamSetupAns);
                    break;
                
                case DEV_STATUS:
                
                    retval = getDevStatusAns(&s, &value.fields.devStatusAns);
                    break;
                
                case NEW_CHANNEL:
                
                    retval = getNewChannelAns(&s, &value.fields.newChannelAns);
                    break;
                    
                case DL_CHANNEL:
                
                    retval = getDLChannelAns(&s, &value.fields.dlChannelAns);
                    break;            
                }
                
                if(retval && (handler != NULL)){
                    
                    handler(receiver, &value);
                }
            }
            else{
                
                LORA_ERROR("cannot recognise MAC command")
                retval = false;
            }
        }
        else{
            
            break;
        }
    }
    
    return retval;
}

/* static functions ***************************************************/


static uint8_t typeToTag(enum lora_mac_cmd_type type)
{
    return tags[type].tag;
}

static bool tagToType(uint8_t tag, enum lora_mac_cmd_type *type)
{
    bool retval = false;
    size_t i;
    
    for(i=0U; i < (sizeof(tags)/sizeof(*tags)); i++){
        
        if(tags[i].tag == tag){
            
            *type = tags[i].type;
            retval = true;
            break;
        }
    }
    
    return retval;
}

static bool getLinkCheckAns(struct lora_stream *s, struct lora_link_check_ans *value)
{
    bool retval = false;
    uint8_t buf;
    
    if(getU8(s, &buf)){
        
        if(getU8(s, &value->gwCount)){
            
            retval = true;
        }
    }
    
    return retval;
}

static bool getLinkADRReq(struct lora_stream *s, struct lora_link_adr_req *value)
{
    bool retval = false;
    
    uint8_t buf;
    
    if(getU8(s, &buf)){
    
        value->dataRate = buf >> 4;
        value->txPower = buf & 0xfU;
    
        if(getU16(s, &value->channelMask)){
                    
            if(getU8(s, &value->redundancy)){
                
                retval = true;                        
            }            
        }                    
    }
    
    return retval;
}

static bool getLinkADRAns(struct lora_stream *s, struct lora_link_adr_ans *value)
{
    bool retval = false;
    
    uint8_t buf;
    
    if(getU8(s, &buf)){
    
        value->powerOK = ((buf & 4U) == 4U);
        value->dataRateOK = ((buf & 2U) == 2U);
        value->channelMaskOK = ((buf & 1U) == 1U);
        
        retval = true;        
    }
    
    return retval;
}

static bool getDutyCycleReq(struct lora_stream *s, struct lora_duty_cycle_req *value)
{
    bool retval = false;
    
    if(getU8(s, &value->maxDutyCycle)){
        
        value->maxDutyCycle = value->maxDutyCycle & 0xfU;        
        retval = true;
    }
    
    return retval;
}

static bool getRXParamSetupReq(struct lora_stream *s, struct lora_rx_param_setup_req *value)
{
    bool retval = false;
    
    if(getU8(s, &value->rx1DROffset)){
        
        if(getU24(s, &value->freq)){
        
            retval = true;        
        }       
    }
    
    return retval;    
}

static bool getRXParamSetupAns(struct lora_stream *s, struct lora_rx_param_setup_ans *value)
{
    bool retval = false;
    
    uint8_t buf;
    
    if(getU8(s, &buf)){
    
        value->rx1DROffsetOK = ((buf & 4U) == 4U);
        value->rx2DataRateOK = ((buf & 2U) == 2U);
        value->channelOK = ((buf & 1U) == 1U);
        
        retval = true;        
    }
    
    return retval;
}

static bool getDevStatusAns(struct lora_stream *s, struct lora_dev_status_ans *value)
{
    bool retval = false;
    
    if(getU8(s, &value->battery)){
        
        if(getU8(s, &value->margin)){
            
            retval = true;
        }
    }
    
    return retval;
}

static bool getNewChannelReq(struct lora_stream *s, struct lora_new_channel_req *value)
{
    bool retval = false;
    uint8_t buf;
    
    if(getU8(s, &value->chIndex)){
        
        if(getU24(s, &value->freq)){
                    
            if(getU8(s, &buf)){

                value->maxDR = buf >> 4;
                value->minDR = buf & 0xfU;                        
                
                retval = true;
            }
        }
    }
    
    return retval;
}

static bool getNewChannelAns(struct lora_stream *s, struct lora_new_channel_ans *value)
{
    bool retval = false;
    
    uint8_t buf;
    
    if(getU8(s, &buf)){
    
        value->dataRateRangeOK = ((buf & 2U) == 2U);
        value->channelFrequencyOK = ((buf & 1U) == 1U);
        
        retval = true;        
    }
    
    return retval;
}

static bool getDLChannelReq(struct lora_stream *s, struct lora_dl_channel_req *value)
{
    bool retval = false;
    
    if(getU8(s, &value->chIndex)){
        
        if(getU24(s, &value->freq)){
        
            retval = true;                                
        }
    }
    
    return retval;
}

static bool getDLChannelAns(struct lora_stream *s, struct lora_dl_channel_ans *value)
{
    bool retval = false;
    
    uint8_t buf;
    
    if(getU8(s, &buf)){
    
        value->channelFrequencyOK = ((buf & 2U) == 2U);
        value->uplinkFreqOK = ((buf & 1U) == 1U);
        
        retval = true;        
    }
    
    return retval;
}

static bool getRXTimingSetupReq(struct lora_stream *s, struct lora_rx_timing_setup_req *value)
{
    bool retval = false;
    
    if(getU8(s, &value->delay)){
            
        value->delay &= 0xfU;        
        retval = true;
    }
    
    return retval;
}
static bool getTXParamSetupReq(struct lora_stream *s, struct lora_tx_param_setup_req *value)
{
    bool retval = false;
    uint8_t buf;
    
    if(getU8(s, &buf)){
            
        value->downlinkDwell = ((buf & 0x20U) == 0x20U); 
        value->uplinkDwell = ((buf & 0x10U) == 0x10U); 
        value->maxEIRP = buf & 0xfU; 
    
        retval = true;
    }
    
    return retval;
}

#if 0

static bool getPingSlotInfoReq(struct lora_stream *s, struct lora_ping_slot_info_req *value)
{
    bool retval = false;
    uint8_t buf;
    
    if(getU8(s, &buf)){
        
        value->periodicity = (buf >> 4U) & 0x7U;
        value->dataRate = buf & 0xfU;
        
        retval = true;
    }
    
    return retval
}

static bool getPingSlotChannelReq(struct lora_stream *s, struct lora_ping_slot_channel_req *value)
{
    bool retval = false;
    
    
    return retval
}

static bool getPingSlotFreqAns(struct lora_stream *s, struct lora_ping_slot_freq_ans *value)
{
    bool retval = false;
    
    
    return retval
}

static bool getBeaconTimingReq(struct lora_stream *s, struct lora_beacon_timing_req *value)
{
    bool retval = false;
    
    
    return retval
}

static bool getBeaconTimingAns(struct lora_stream *s, struct lora_beacon_timing_ans *value)
{
    bool retval = false;
    
    
    return retval
}

static bool getBeaconFreqReq(struct lora_stream *s, struct lora_beacon_freq_req *value)
{
    bool retval = false;
    
    
    return retval
}

static bool getBeaconFreqAns(struct lora_stream *s, struct lora_beacon_freq_ans *value)
{
    bool retval = false;
    
    
    return retval
}

#endif

static bool putU24(struct lora_stream *self, uint32_t in)
{
    uint8_t out[] = {
        in, 
        in >> 8,
        in >> 16
    };
    
    return Stream_write(self, out, sizeof(out));
}

static bool putU16(struct lora_stream *self, uint16_t in)
{
    uint8_t out[] = {
        in, 
        in >> 8        
    };
    
    return Stream_write(self, out, sizeof(out));
}

static bool putU8(struct lora_stream *self, uint8_t in)
{
    return Stream_write(self, &in, sizeof(in));
}

static bool getU24(struct lora_stream *self, uint32_t *out)
{
    uint8_t buf[3U];
    bool retval;
    
    *out = 0U;
    
    retval = Stream_read(self, buf, sizeof(buf));
        
    *out |= buf[2];
    *out <<= 8;
    *out |= buf[1];
    *out <<= 8;
    *out |= buf[0];
    
    return retval;
}

static bool getU16(struct lora_stream *self, uint16_t *out)
{
    uint8_t buf[2U];
    bool retval;
    
    *out = 0U;
    
    retval = Stream_read(self, buf, sizeof(buf));
        
    *out |= buf[1];
    *out <<= 8;
    *out |= buf[0];
        
    return retval;
}

static bool getU8(struct lora_stream *self, uint8_t *out)
{
    return Stream_read(self, out, sizeof(*out));
}
