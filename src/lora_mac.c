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
#include "lora_time.h"

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

/* functions **********************************************************/

void LoraMAC_init(struct lora_mac *self, struct lora_channel_list *channels, struct lora_radio *radio, struct lora_event *events)
{
    LORA_ASSERT(self != NULL)
    LORA_ASSERT(channels != NULL)
    LORA_ASSERT(radio != NULL)
    LORA_ASSERT(events != NULL)

    (void)memset(self, 0, sizeof(*self));

    self->channels = channels;
    self->radio = radio;    
    self->events = events;
    
    const struct region_defaults *defaults = LoraRegion_getDefaultSettings(ChannelList_region(self->channels));
    
    LoraRadio_setEventHandler(self->radio, self, radioEvent);
    
    self->maxFrameCounterGap = defaults->max_fcnt_gap;    
    self->joinAcceptDelay1 = defaults->join_accept_delay1 * 1000U;
    self->joinAcceptDelay2 = defaults->join_accept_delay2 * 1000U;    
    self->receiveDelay1 = defaults->receive_delay1;
    self->receiveDelay2 = defaults->receive_delay2;    
    //self->adrAckDelay = defaults->adr_ack_delay;
    //self->adrAckLimit = defaults->adr_ack_limit;
    //self->ackTimeout = defaults->ack_timeout;
    //self->ackDither = defautls->ack_dither;
}

bool LoraMAC_send(struct lora_mac *self, bool confirmed, uint8_t port, const void *data, uint8_t len)
{
    LORA_ASSERT(self != NULL)
    LORA_ASSERT((len == 0) || (data != NULL))
    
    bool retval = false;
    uint16_t bufferMax = LoraFrame_getPhyPayloadSize(0, len);
    
    struct lora_channel_setting settings;
    
    /* stack must be idle (i.e. not sending) */
    if(self->state == IDLE){
    
        if((port > 0U) && (port <= 223U)){

            if(ChannelList_upstreamSetting(self->channels, &settings)){
                
                if(settings.maxPayload >= bufferMax){ 

                    struct lora_frame f = {

                        .type = FRAME_TYPE_DATA_UNCONFIRMED_UP,
                        .fields.data.devAddr = self->devAddr,
                        .fields.data.counter = (uint32_t)getUpCount(self),
                        .fields.data.ack = false,
                        .fields.data.adr = false,
                        .fields.data.adrAckReq = false,
                        .fields.data.pending = false,
                        .fields.data.opts = NULL,
                        .fields.data.optsLen = 0U,
                        .fields.data.port = port,
                        .fields.data.data = ((len > 0U) ? (const uint8_t *)data : NULL),
                        .fields.data.dataLen = len
                    };

                    self->bufferLen = LoraFrame_encode(self->appSKey, &f, self->buffer, sizeof(self->buffer));
                    
                    uint64_t timeNow = getTime();
                    
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
    
    return retval;
}

void LoraMAC_setReceiveHandler(struct lora_mac *self, void *receiver, rxCompleteCB cb)
{
    self->rxCompleteHandler = cb;
    self->rxCompleteReceiver = receiver;
}

void LoraMAC_setTransmitHandler(struct lora_mac *self, void *receiver, txCompleteCB cb)
{
    self->txCompleteHandler = cb;
    self->txCompleteReceiver = receiver;
}

bool LoraMAC_setNbTrans(struct lora_mac *self, uint8_t nbTrans)
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

bool LoraMAC_join(struct lora_mac *self)
{
    bool retval;
    
    if(self->state == IDLE){

        struct lora_frame f;
        
        f.type = FRAME_TYPE_JOIN_REQ;
        
        (void)memcpy(f.fields.joinRequest.appEUI, self->appEUI, sizeof(f.fields.joinRequest.appEUI));
        (void)memcpy(f.fields.joinRequest.appEUI, self->devEUI, sizeof(f.fields.joinRequest.devEUI));
        f.fields.joinRequest.devNonce = 0U;

        self->bufferLen = LoraFrame_encode(self->appKey, &f, self->buffer, sizeof(self->buffer));
        
        uint64_t timeNow = getTime();
        
        (void)Event_onTimeout(self->events, timeNow + ChannelList_waitTime(self->channels, timeNow), self, tx);
        
        self->state = JOIN_WAIT_TX;
        self->joinPending = true;
        
        retval = true;        
    }
    
    return retval;
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
    struct lora_channel_setting settings;
    
    LORA_ASSERT((self->state == JOIN_WAIT_TX) || (self->state == WAIT_TX))
    
    if(ChannelList_upstreamSetting(self->channels, &settings)){
    
        if(LoraRadio_setParameters(self->radio, settings.freq, settings.bw, settings.sf)){
        
            self->state = TX;
            
            self->txComplete = Event_onInput(self->events, EVENT_TX_COMPLETE, self, txComplete);
        
            self->resetRadio = NULL;    //todo: add a watchdog to catch situation where IO lines don't work as expected
            
            LoraRadio_transmit(self->radio, self->buffer, self->bufferLen);
        }
        else{
            
            LORA_ERROR("radio reject parameters")            
            abandonSequence(self);
        }
    }
    else{
        
        LORA_ERROR("no upstream radio parameters available")
        abandonSequence(self);
    }
}

static void txComplete(void *receiver, uint64_t time)
{
    LORA_ASSERT(receiver != NULL)
    
    struct lora_mac *self = (struct lora_mac *)receiver;            
    uint64_t timeNow = getTime();
    uint64_t futureTime;
        
    LORA_ASSERT((self->state == JOIN_TX) || (self->state == TX))
    
    self->txCompleteTime = time;
        
    futureTime = self->txCompleteTime + ((self->state == TX) ? self->receiveDelay1 : self->joinAcceptDelay1);
    
    self->state = (self->state == TX) ? WAIT_RX1 : JOIN_WAIT_RX1;                

    if(futureTime > timeNow){

        (void)Event_onTimeout(self->events, futureTime, self, rxStart);
    }
    else{
        
        LORA_ERROR("missed RX slot")
        
        switch(self->state){
        case RX1:
        case JOIN_RX1:
            ChannelList_registerTransmission(self->channels, self->txCompleteTime, self->bufferLen);
            break;    
        default:
            break;
        }
        
        rxFinish(self);
    }         
}

static void rxStart(void *receiver, uint64_t time)
{
    LORA_ASSERT(receiver != NULL)
    
    struct lora_mac *self = (struct lora_mac *)receiver;    
    struct lora_channel_setting settings;
        
    LORA_ASSERT((self->state == WAIT_RX1) || (self->state == WAIT_RX2) || (self->state == JOIN_WAIT_RX1) || (self->state == JOIN_WAIT_RX2))
        
    switch(self->state){
    default:
    case JOIN_WAIT_RX1:
    
        (void)ChannelList_rx1Setting(self->channels, &settings);
        self->state = RX1;                
        break;
    
    case WAIT_RX1:
    
        (void)ChannelList_rx1Setting(self->channels, &settings);
        self->state = JOIN_RX1;                
        break;
    
    case JOIN_WAIT_RX2:    
    
        (void)ChannelList_rx2Setting(self->channels, &settings);
        self->state = RX2;                
        break;        
    
    case WAIT_RX2:
    
        (void)ChannelList_rx2Setting(self->channels, &settings);
        self->state = JOIN_RX2;                
        break;                    
    }
    
    if(LoraRadio_setParameters(self->radio, settings.freq, settings.bw, settings.sf)){
     
        self->rxReady = Event_onInput(self->events, EVENT_RX_READY, self, rxReady);
        self->rxTimeout = Event_onInput(self->events, EVENT_RX_TIMEOUT, self, rxTimeout);
        
        LoraRadio_receive(self->radio);                                
    }
    else{
        
        LORA_ERROR("could not apply radio settings")
        abandonSequence(self);
    }
}

static void rxReady(void *receiver, uint64_t time)
{
    LORA_ASSERT(receiver != NULL)
        
    struct lora_mac *self = (struct lora_mac *)receiver;    
    Event_cancel(self->events, &self->rxTimeout);    
    
    LORA_ASSERT((self->state == RX1) || (self->state == RX2) || (self->state == JOIN_RX1) || (self->state == JOIN_RX2))
    
    switch(self->state){
    case RX1:
    case JOIN_RX1:
        ChannelList_registerTransmission(self->channels, self->txCompleteTime, self->bufferLen);
        break;    
    default:
        break;
    }
    
    collect(self);            
    rxFinish(self);
}

static void rxTimeout(void *receiver, uint64_t time)
{
    LORA_ASSERT(receiver != NULL)
    
    struct lora_mac *self = (struct lora_mac *)receiver;    
    Event_cancel(self->events, &self->rxReady);    
    
    LORA_ASSERT((self->state == RX1) || (self->state == RX2) || (self->state == JOIN_RX1) || (self->state == JOIN_RX2))
    
    switch(self->state){
    case RX1:
    case JOIN_RX1:
        ChannelList_registerTransmission(self->channels, self->txCompleteTime, self->bufferLen);
        break;    
    default:
        break;
    }
    
    rxFinish(self);
}

static void rxFinish(struct lora_mac *self)
{    
    LORA_ASSERT(self != NULL)
    
    switch(self->state){
    default:
    case RX1:        
        self->state = WAIT_RX2;    
        (void)Event_onTimeout(self->events, self->txCompleteTime + self->receiveDelay2, self, rxStart);
        break;
    
    case JOIN_RX1:        
        self->state = JOIN_WAIT_RX2;    
        (void)Event_onTimeout(self->events, self->txCompleteTime + self->joinAcceptDelay2, self, rxStart);
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
    
    LoraRadio_reset(self->radio);
    
    LORA_MSG("radio has been reset")
    
    Event_cancel(self->events, &self->rxReady);
    self->rxReady = NULL;
    
    Event_cancel(self->events, &self->rxTimeout);
    self->rxTimeout = NULL;
    
    Event_cancel(self->events, &self->txComplete);
    self->txComplete = NULL;    
    
    abandonSequence(self);
}
#endif
    
static void collect(struct lora_mac *self)
{
    struct lora_frame result;
    
    self->bufferLen = LoraRadio_collect(self->radio, self->buffer, sizeof(self->buffer));        
    
    if(LoraFrame_decode(self->appKey, self->nwkSKey, self->appSKey, self->buffer, self->bufferLen, &result)){
        
        switch(result.type){
        case FRAME_TYPE_JOIN_ACCEPT:
        
            if(self->joinPending){
                
                struct lora_aes_ctx aes_ctx;
                struct lora_cmac_ctx cmac_ctx;
                uint8_t hdr;
                
                LoraAES_init(&aes_ctx, self->appKey);
                LoraCMAC_init(&cmac_ctx, &aes_ctx); 
                
                hdr = 1U;
                LoraCMAC_update(&cmac_ctx, &hdr, sizeof(hdr));
                LoraCMAC_update(&cmac_ctx, result.fields.joinAccept.appNonce, sizeof(result.fields.joinAccept.appNonce));
                LoraCMAC_update(&cmac_ctx, result.fields.joinAccept.netID, sizeof(result.fields.joinAccept.netID));
                LoraCMAC_update(&cmac_ctx, &self->devNonce, sizeof(self->devNonce));
                LoraCMAC_finish(&cmac_ctx, self->nwkSKey, sizeof(self->nwkSKey));
                
                hdr = 2U;
                LoraCMAC_update(&cmac_ctx, &hdr, sizeof(hdr));
                LoraCMAC_update(&cmac_ctx, result.fields.joinAccept.appNonce, sizeof(result.fields.joinAccept.appNonce));
                LoraCMAC_update(&cmac_ctx, result.fields.joinAccept.netID, sizeof(result.fields.joinAccept.netID));
                LoraCMAC_update(&cmac_ctx, &self->devNonce, sizeof(self->devNonce));
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
                            //LoraMAC_processCommands(self, result.opts, result.optsLen);
                        }
                        
                        if(result.fields.data.port == 0U){
                                        
                            //LoraMAC_processCommands(self, result.data, result.dataLen);
                        }
                        else{
                            
                            if(self->rxCompleteHandler){
                                
                                self->rxCompleteHandler(self->rxCompleteReceiver, result.fields.data.port, result.fields.data.data, result.fields.data.dataLen);
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


