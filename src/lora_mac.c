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

#include "lora_mac.h"
#include "lora_channel_list.h"
#include "lora_radio.h"
#include "lora_event.h"
#include "lora_frame.h"
#include "lora_debug.h"
#include "lora_aes.h"
#include "lora_cmac.h"
#include "lora_system.h"
#include "lora_region.h"
#include "lora_persistent.h"

#include <string.h>

/* types **************************************************************/


/* static function prototypes *****************************************/

static uint16_t getUpCount(struct lora_mac *self);
static bool validateDownCount(struct lora_mac *self, uint16_t counter);

static void tx(void *receiver, uint64_t time);
static void txComplete(void *receiver, uint64_t time);

static void rxStart(void *receiver, uint64_t time);
static void rxReady(void *receiver, uint64_t time);
static void rxTimeout(void *receiver, uint64_t time);
static void rxFinish(struct lora_mac *self);

//static void resetRadio(void *receiver, uint64_t time);
static void collect(struct lora_mac *self);

static void abandonSequence(struct lora_mac *self);

static void radioEvent(void *receiver, enum lora_radio_event event, uint64_t time);

static uint32_t calculateOnAirTime(const struct lora_mac *self, enum lora_signal_bandwidth bw, enum lora_spreading_factor sf, enum lora_coding_rate cr, uint8_t payloadLen);

/* functions **********************************************************/

void MAC_init(struct lora_mac *self, struct lora_channel_list *channels, struct lora_radio *radio, struct lora_event *events)
{
    LORA_ASSERT(self != NULL)
    LORA_ASSERT(channels != NULL)
    LORA_ASSERT(radio != NULL)
    LORA_ASSERT(events != NULL)

    (void)memset(self, 0, sizeof(*self));

    self->channels = channels;
    self->radio = radio;    
    self->events = events;
    
    const struct lora_region_default *defaults = Region_getDefaultSettings(ChannelList_region(self->channels));
    
    Radio_setEventHandler(self->radio, self, radioEvent);

    self->rx2_rate = defaults->rx2_rate;
    self->rx2_freq = defaults->rx2_freq;
    self->power = defaults->init_tx_power;
    self->rate = defaults->init_tx_rate;    
    
    self->maxFrameCounterGap = defaults->max_fcnt_gap;    
    self->ja1_delay = defaults->ja1_delay * 1000000U;
    self->ja2_delay = defaults->ja2_delay * 1000000U;    
    self->rx1_delay = defaults->rx1_delay * 1000000U;
    self->rx2_delay = defaults->rx2_delay * 1000000U;    
    //self->adrAckDelay = defaults->adr_ack_delay;
    //self->adrAckLimit = defaults->adr_ack_limit;
    //self->ackTimeout = defaults->ack_timeout;
    //self->ackDither = defautls->ack_dither;
    
    Persistent_getAppEUI(self, self->appEUI);
    Persistent_getDevEUI(self, self->devEUI);
    Persistent_getAppKey(self, self->appKey);
}

bool MAC_send(struct lora_mac *self, bool confirmed, uint8_t port, const void *data, uint8_t len)
{
    LORA_ASSERT(self != NULL)
    LORA_ASSERT((len == 0) || (data != NULL))
    
    bool retval = false;
    uint16_t bufferMax = Frame_getPhyPayloadSize(0, len);
    
    struct lora_channel_setting settings;
    const struct lora_data_rate *rate_setting;
    struct lora_frame f;
    
    if(self->personalised || self->joined){
    
        /* stack must be idle (i.e. not sending) */
        if(self->state == IDLE){
        
            if((port > 0U) && (port <= 223U)){

                if(ChannelList_txSetting(self->channels, &settings)){
                    
                    rate_setting = Region_getDataRateParameters(ChannelList_region(self->channels), self->rate);
                    
                    LORA_ASSERT(rate_setting != NULL)
                    
                    if(rate_setting->payload >= bufferMax){ 

                        f.type = FRAME_TYPE_DATA_UNCONFIRMED_UP;
                        f.fields.data.devAddr = self->devAddr;
                        f.fields.data.counter = (uint32_t)getUpCount(self);
                        f.fields.data.ack = false;
                        f.fields.data.adr = false;
                        f.fields.data.adrAckReq = false;
                        f.fields.data.pending = false;
                        f.fields.data.opts = NULL;
                        f.fields.data.optsLen = 0U;
                        f.fields.data.port = port;
                        f.fields.data.data = ((len > 0U) ? (const uint8_t *)data : NULL);
                        f.fields.data.dataLen = len;
                        
                        self->bufferLen = Frame_encode((port == 0) ? self->nwkSKey : self->appSKey, &f, self->buffer, sizeof(self->buffer));
                        
                        uint64_t timeNow = System_getTime();
                        
                        (void)Event_onTimeout(self->events, timeNow + ChannelList_waitTime(self->channels, timeNow), self, tx);
                        
                        self->state = WAIT_TX;
                        
                        self->confirmPending = true;
                        self->confirmed = false;
                        
                        retval = true;                    
                    }
                    else{
                        
                        LORA_ERROR("payload too large")
                    }
                }
                else{
                    
                    LORA_ERROR("no channels")
                }
            }
            else{
                
                LORA_ERROR("application port must be in range 1..223")
            }
        }
        else{
            
            LORA_ERROR("MAC is busy")
        }
    }
    else{
        
        LORA_ERROR("stack must be personalised or joined before sending data")
    }
    
    return retval;
}

bool MAC_join(struct lora_mac *self)
{
    bool retval;

    if(!self->personalised){
    
        if(self->state == IDLE){

            struct lora_frame f;            
            
            f.type = FRAME_TYPE_JOIN_REQ;
            
            (void)memcpy(f.fields.joinRequest.appEUI, self->appEUI, sizeof(f.fields.joinRequest.appEUI));
            (void)memcpy(f.fields.joinRequest.appEUI, self->devEUI, sizeof(f.fields.joinRequest.devEUI));
            f.fields.joinRequest.devNonce = 0U;

            self->bufferLen = Frame_encode(self->appKey, &f, self->buffer, sizeof(self->buffer));
            
            uint64_t timeNow = System_getTime();
            
            (void)Event_onTimeout(self->events, timeNow + ChannelList_waitTime(self->channels, timeNow), self, tx);
            
            self->state = JOIN_WAIT_TX;
            self->joinPending = true;
            
            retval = true;        
        }
    }
    else{
        
        LORA_ERROR("stack has been personalised - join is not possible")
    }   
    
    return retval;
}

void MAC_setResponseHandler(struct lora_mac *self, void *receiver, lora_mac_response_fn cb)
{
    self->responseHandler = cb;
    self->responseReceiver = receiver;
}

bool MAC_setNbTrans(struct lora_mac *self, uint8_t nbTrans)
{
    bool retval = false;
    
    if(self->state == IDLE){
        
        if(nbTrans <= 0xf){
        
            self->nbTrans = nbTrans;
            retval = true;
        }        
    }
    
    return retval;
}

bool MAC_personalize(struct lora_mac *self, uint32_t devAddr, const void *nwkSKey, const void *appSKey)
{
    self->devAddr = devAddr;
    (void)memcpy(self->nwkSKey, nwkSKey, sizeof(self->nwkSKey));
    (void)memcpy(self->appSKey, appSKey, sizeof(self->appSKey));    
    return true;
}

/* static functions ***************************************************/

static uint16_t getUpCount(struct lora_mac *self)
{
    self->upCounter++;
    return self->upCounter;
}

static bool validateDownCount(struct lora_mac *self, uint16_t counter)
{
    bool retval = false;
    
    if((uint32_t)counter < ((uint32_t)self->downCounter + (uint32_t)self->maxFrameCounterGap)){
        
        retval = true;
        self->downCounter = counter;
    }
    
    return retval;
}

static void resetCounters(struct lora_mac *self)
{
    self->upCounter = 0U;
    self->downCounter = 0U;
}

static void tx(void *receiver, uint64_t time)
{
    LORA_ASSERT(receiver != NULL)
    
    struct lora_mac *self = (struct lora_mac *)receiver;    
    struct lora_channel_setting channel_setting;
    struct lora_radio_tx_setting radio_setting;
    const struct lora_data_rate *rate_setting;
    uint32_t transmitTime;
    
    LORA_ASSERT((self->state == JOIN_WAIT_TX) || (self->state == WAIT_TX))
    
    if(ChannelList_txSetting(self->channels, &channel_setting)){

        rate_setting = Region_getDataRateParameters(ChannelList_region(self->channels), self->rate);

        radio_setting.freq = channel_setting.freq;
        radio_setting.bw = rate_setting->bw;
        radio_setting.sf = rate_setting->sf;
        radio_setting.cr = CR_5;
        radio_setting.power = self->power;
        
        if(Radio_transmit(self->radio, &radio_setting, self->buffer, self->bufferLen)){
    
            self->previousRate = self->rate;
            self->previousFreq = channel_setting.freq;
            
            transmitTime = calculateOnAirTime(self, radio_setting.bw, radio_setting.sf, radio_setting.cr, self->bufferLen);
            
            ChannelList_registerTransmission(self->channels, System_getTime(), transmitTime);
            
            self->state = TX;
                
            self->txComplete = Event_onInput(self->events, EVENT_TX_COMPLETE, self, txComplete);
        
            self->resetRadio = NULL;    //todo: add a watchdog to catch situation where IO lines don't work as expected    
        }
        else{
            
            LORA_ERROR("radio reject parameters")            
            abandonSequence(self);
        }
    }
    else{
        
        LORA_ERROR("no tx radio parameters available")
        abandonSequence(self);
    }
}

static void txComplete(void *receiver, uint64_t time)
{
    LORA_ASSERT(receiver != NULL)
    
    struct lora_mac *self = (struct lora_mac *)receiver;            
    uint64_t timeNow = System_getTime();
    uint64_t futureTime;
        
    LORA_ASSERT((self->state == JOIN_TX) || (self->state == TX))
    
    Radio_sleep(self->radio);
    
    self->txCompleteTime = time;
        
    futureTime = self->txCompleteTime + ((self->state == TX) ? self->rx1_delay : self->ja1_delay);
    
    self->state = (self->state == TX) ? WAIT_RX1 : JOIN_WAIT_RX1;                

    if(futureTime > timeNow){

        (void)Event_onTimeout(self->events, futureTime, self, rxStart);
    }
    else{
        
        LORA_ERROR("missed RX slot")        
        rxFinish(self);
    }         
}

static void rxStart(void *receiver, uint64_t time)
{
    LORA_ASSERT(receiver != NULL)
    
    struct lora_mac *self = (struct lora_mac *)receiver;    
    struct lora_radio_rx_setting radio_setting;
    const struct lora_data_rate *rate_setting;
    uint8_t rate;
    uint64_t targetTime;
    uint64_t timeNow = System_getTime();
        
    LORA_ASSERT((self->state == WAIT_RX1) || (self->state == WAIT_RX2) || (self->state == JOIN_WAIT_RX1) || (self->state == JOIN_WAIT_RX2))
    LORA_ASSERT(time <= timeNow)
   
    targetTime = timeNow - self->rx1_delay;
    
    if(targetTime <= self->txCompleteTime){
        
        /* wait until the deadline - todo: add a faff margin */
        if(targetTime < self->txCompleteTime){
            
            System_usleep(self->txCompleteTime - targetTime);
        }
        
        switch(self->state){
        default:
        case JOIN_WAIT_RX1:
        case WAIT_RX1:
        
            if(Region_getRX1DataRate(ChannelList_region(self->channels), self->previousRate, self->rx1_offset, &rate)){
            
                radio_setting.freq = self->previousFreq;
                
                self->state = (self->state == WAIT_RX1) ? RX1 : JOIN_RX1;                            
            }
            break;
        
        case JOIN_WAIT_RX2:    
        case WAIT_RX2:    
        
            rate = self->rx2_rate;
            radio_setting.freq = self->rx2_freq;
            self->state = (self->state == WAIT_RX2) ? RX2 : JOIN_RX2;                
            break;        
        }
        
        rate_setting = Region_getDataRateParameters(ChannelList_region(self->channels), rate);
        
        radio_setting.bw = rate_setting->bw;
        radio_setting.sf = rate_setting->sf;
        radio_setting.cr = CR_5;
        radio_setting.preamble = 8U;
        
        if(Radio_receive(self->radio, &radio_setting)){
         
            self->rxReady = Event_onInput(self->events, EVENT_RX_READY, self, rxReady);
            self->rxTimeout = Event_onInput(self->events, EVENT_RX_TIMEOUT, self, rxTimeout);                            
        }
        else{
            
            LORA_ERROR("could not apply radio settings")
            abandonSequence(self);
        }
    }
    /* missed deadline */
    else{
        
        switch(self->state){
        default:
        case JOIN_WAIT_RX1:
            LORA_ERROR("missed JOIN RX1 deadline")
            self->state = JOIN_RX1;
            break;
            
        case WAIT_RX1:
            LORA_ERROR("missed RX1 deadline")
            self->state = RX1;
            break;
            
        case JOIN_WAIT_RX2:    
            LORA_ERROR("missed JOIN RX2 deadline")
            self->state = JOIN_RX2;
            break;
        
        case WAIT_RX2:    
            LORA_ERROR("missed RX2 deadline")
            self->state = RX2;
            break;
        }
        
        rxFinish(self);
    }
}

static void rxReady(void *receiver, uint64_t time)
{
    LORA_ASSERT(receiver != NULL)
        
    struct lora_mac *self = (struct lora_mac *)receiver;    
    Event_cancel(self->events, &self->rxTimeout);    
    
    LORA_ASSERT((self->state == RX1) || (self->state == RX2) || (self->state == JOIN_RX1) || (self->state == JOIN_RX2))
    
    collect(self);            
    rxFinish(self);
}

static void rxTimeout(void *receiver, uint64_t time)
{
    LORA_ASSERT(receiver != NULL)
    
    struct lora_mac *self = (struct lora_mac *)receiver;    
    Event_cancel(self->events, &self->rxReady);    
    
    LORA_ASSERT((self->state == RX1) || (self->state == RX2) || (self->state == JOIN_RX1) || (self->state == JOIN_RX2))
    
    rxFinish(self);
}

static void rxFinish(struct lora_mac *self)
{    
    LORA_ASSERT(self != NULL)
    
    Radio_sleep(self->radio);
    
    switch(self->state){
    default:
    case RX1:        
        self->state = WAIT_RX2;    
        (void)Event_onTimeout(self->events, self->txCompleteTime + self->rx2_delay, self, rxStart);
        break;
    
    case JOIN_RX1:        
        self->state = JOIN_WAIT_RX2;    
        (void)Event_onTimeout(self->events, self->txCompleteTime + self->ja2_delay, self, rxStart);
        break;
        
    case RX2:        
        //todo retries
        //todo retransmissions
        self->state = IDLE;            
        break;
    
    case JOIN_RX2:   
        //todo retries
        self->state = IDLE;            
        break;
    }        
}

#if 0
static void resetRadio(void *receiver, uint64_t time)
{
    LORA_ASSERT(receiver != NULL)
    
    struct lora_mac *self = (struct lora_mac *)receiver;    
    uint32_t delay;
    
    LORA_MSG("radio has been reset")
    
    Event_cancel(self->events, &self->rxReady);
    self->rxReady = NULL;
    
    Event_cancel(self->events, &self->rxTimeout);
    self->rxTimeout = NULL;
    
    Event_cancel(self->events, &self->txComplete);
    self->txComplete = NULL;    
    
    delay = Radio_resetHardware(self->radio);
    
    abandonSequence(self);
}
#endif
    
static void collect(struct lora_mac *self)
{
    struct lora_frame result;
    
    self->bufferLen = Radio_collect(self->radio, self->buffer, sizeof(self->buffer));        
    
    if(Frame_decode(self->appKey, self->nwkSKey, self->appSKey, self->buffer, self->bufferLen, &result)){
        
        switch(result.type){
        case FRAME_TYPE_JOIN_ACCEPT:
        
            if(self->joinPending){
                
                struct lora_aes_ctx aes_ctx;
                struct lora_cmac_ctx cmac_ctx;
                uint8_t block[16U];
                
                LoraAES_init(&aes_ctx, self->appKey);
                
                block[0] = 1U;
                block[1] = result.fields.joinAccept.appNonce;
                block[2] = result.fields.joinAccept.appNonce >> 8;
                block[3] = result.fields.joinAccept.appNonce >> 16;
                block[4] = result.fields.joinAccept.netID;
                block[5] = result.fields.joinAccept.netID >> 8;
                block[6] = result.fields.joinAccept.netID >> 16;
                block[7] = self->devNonce;
                block[8] = self->devNonce >> 8;                    
                block[9] = 0U;                    
                block[10] = 0U;                    
                block[11] = 0U;                    
                block[12] = 0U;                    
                block[13] = 0U;                    
                block[14] = 0U;                    
                block[15] = 0U;                    
                
                LoraCMAC_init(&cmac_ctx, &aes_ctx); 
                LoraCMAC_update(&cmac_ctx, block, sizeof(block));
                LoraCMAC_finish(&cmac_ctx, self->nwkSKey, sizeof(self->nwkSKey));
                
                block[0] = 2U;
                block[1] = result.fields.joinAccept.appNonce;
                block[2] = result.fields.joinAccept.appNonce >> 8;
                block[3] = result.fields.joinAccept.appNonce >> 16;
                block[4] = result.fields.joinAccept.netID;
                block[5] = result.fields.joinAccept.netID >> 8;
                block[6] = result.fields.joinAccept.netID >> 16;
                block[7] = self->devNonce;
                block[8] = self->devNonce >> 8;                    
                block[9] = 0U;                    
                block[10] = 0U;                    
                block[11] = 0U;                    
                block[12] = 0U;                    
                block[13] = 0U;                    
                block[14] = 0U;                    
                block[15] = 0U;                    
                
                LoraCMAC_init(&cmac_ctx, &aes_ctx); 
                LoraCMAC_update(&cmac_ctx, block, sizeof(block));
                LoraCMAC_finish(&cmac_ctx, self->appSKey, sizeof(self->appSKey));
                
                self->joinPending = false;
                self->joined = true;                
                
                resetCounters(self);                
            }
            else{
                
                LORA_ERROR("ignoring a JOIN Accept we didn't ask for")
            }
            break;
        
        case FRAME_TYPE_DATA_UNCONFIRMED_DOWN:
        case FRAME_TYPE_DATA_CONFIRMED_DOWN:
            
            if(self->personalised){
            
                if(self->devAddr == result.fields.data.devAddr){
                
                    if(validateDownCount(self, result.fields.data.counter)){
                
                        if(result.fields.data.optsLen > 0U){
                            
                            //process mac layer
                            //MAC_processCommands(self, result.opts, result.optsLen);
                        }
                        
                        if(result.fields.data.port == 0U){
                                        
                            //MAC_processCommands(self, result.data, result.dataLen);
                        }
                        else{
                            
                            if(self->responseHandler != NULL){
                                
                                union lora_mac_response_arg arg;
                                
                                arg.rx.data = result.fields.data.data;
                                arg.rx.len = result.fields.data.dataLen;
                                arg.rx.port = result.fields.data.port;
                                
                                self->responseHandler(self->responseReceiver, LORA_MAC_RX, &arg);
                            }
                        }
                    }
                    else{
                        
                        LORA_ERROR("discarding frame for counter sync")
                    }
                }
            }
            break;
        
        default:
            break;
        }
    }
}

static void abandonSequence(struct lora_mac *self)
{
    switch(self->state){
    case WAIT_TX:
    case TX:
    case WAIT_RX1:
    case WAIT_RX2:
        break;
    case JOIN_WAIT_TX:
    case JOIN_TX:
    case JOIN_WAIT_RX1:
    case JOIN_WAIT_RX2:
        break;
    default:
        break;
    }
    
    self->state = IDLE;
}

static void radioEvent(void *receiver, enum lora_radio_event event, uint64_t time)
{
    LORA_ASSERT(receiver != NULL)
    
    struct lora_mac *self = (struct lora_mac *)receiver;
    
    switch(event){
    case LORA_RADIO_TX_COMPLETE:
        Event_receive(self->events, EVENT_TX_COMPLETE, time);
        break;
    case LORA_RADIO_RX_READY:
        Event_receive(self->events, EVENT_RX_READY, time);
        break;
    case LORA_RADIO_RX_TIMEOUT:
        Event_receive(self->events, EVENT_RX_TIMEOUT, time);
        break;
    default:
        break;
    }
}

static uint32_t calculateOnAirTime(const struct lora_mac *self, enum lora_signal_bandwidth bw, enum lora_spreading_factor sf, enum lora_coding_rate cr, uint8_t payloadLen)
{
    /* from 4.1.1.7 of sx1272 datasheet
     *
     * Ts (symbol period)
     * Rs (symbol rate)
     * PL (payload length)
     * SF (spreading factor
     * CRC (presence of trailing CRC)
     * IH (presence of implicit header)
     * DE (presence of data rate optimize)
     * CR (coding rate 1..4)
     * 
     *
     * Ts = 1 / Rs
     * Tpreamble = ( Npreamble x 4.25 ) x Tsym
     *
     * Npayload = 8 + max( ceil[( 8PL - 4SF + 28 + 16CRC + 20IH ) / ( 4(SF - 2DE) )] x (CR + 4), 0 )
     *
     * Tpayload = Npayload x Ts
     *
     * Tpacket = Tpreamble + Tpayload
     *
     * Implementation details:
     *
     * - period will be in microseconds so we can use integer operations rather than float
     * 
     * */

    uint32_t Tpacket = 0U;

    if((bw != BW_FSK) && (sf != SF_FSK)){

        // for now hardcode this according to this recommendation
        bool lowDataRateOptimize = ((bw == BW_125) && ((sf == SF_11) || (sf == SF_12))) ? true : false;
        bool crc = true;    // true for uplink, false for downlink
        bool header = true; 

        uint32_t Ts = ((1U << sf) * 1000000U) / bw;     //symbol rate (us)
        uint32_t Tpreamble = (Ts * 12U) +  (Ts / 4U);       //preamble (us)

        uint32_t numerator = (8U * (uint32_t)payloadLen) - (4U * (uint32_t)sf) + 28U + ( crc ? 16U : 0U ) - ( (header) ? 20U : 0U );
        uint32_t denom = 4U * ((uint32_t)sf - ( lowDataRateOptimize ? 2U : 0U ));

        uint32_t Npayload = 8U + ((numerator / denom) + (((numerator % denom) != 0) ? 1U : 0U)) * ((uint32_t)cr + 4U);

        uint32_t Tpayload = Npayload * Ts;

        Tpacket = Tpreamble + Tpayload;
    }

    return Tpacket;
}

