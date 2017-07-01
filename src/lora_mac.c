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


#include <string.h>

/* types **************************************************************/


/* static function prototypes *****************************************/

static uint32_t upCount(struct lora_mac *self);

static void tx(void *receiver, uint32_t time);
static void txComplete(void *receiver, uint32_t time);

static void rx1Start(void *receiver, uint32_t time);
static void rx1Ready(void *receiver, uint32_t time);
static void rx1Timeout(void *receiver, uint32_t time);
static void rx1Finish(struct lora_mac *self, uint32_t time);

static void rx2Start(void *receiver, uint32_t time);
static void rx2Ready(void *receiver, uint32_t time);
static void rx2Timeout(void *receiver, uint32_t time);
static void rx2Finish(struct lora_mac *self, uint32_t time);

static void resetRadio(void *receiver, uint32_t time);
static void collect(struct lora_mac *self);

#if 0
static void beaconStart(void *receiver, uint32_t time);
static void beaconReady(void *receiver, uint32_t time);
static void beaconTimeout(void *receiver, uint32_t time);
static void beaconFinish(struct lora_mac *self, uint32_t time);
#endif

static void radioEvent(void *receiver, enum lora_radio_event event, uint32_t time);

/* functions **********************************************************/

void LoraMAC_init(struct lora_mac *self, struct lora_channel_list *channels, struct lora_radio *radio, struct lora_event *events)
{
    LORA_ASSERT(self != NULL)
    LORA_ASSERT(channels != NULL)
    LORA_ASSERT(radio != NULL)
    LORA_ASSERT(event != NULL)

    (void)memset(self, 0, sizeof(*self));

    self->channels = channels;
    self->radio = radio;    
    self->events = events;
    
    const struct region_defaults *defaults = LoraRegion_getDefaultSettings(ChannelList_region(self->channels));
    
    self->rx1_interval = defaults->receive_delay1 * 1000U;
    self->rx2_interval = defaults->receive_delay2 * 1000U;
    
    LoraRadio_setEventHandler(self->radio, self, radioEvent);
}

void LoraMAC_setSession(struct lora_mac *self, uint32_t devAddr, const void *nwkSKey, const void *appSKey)
{
    LORA_ASSERT(self != NULL)
    LORA_ASSERT(nwkSKey != NULL)
    LORA_ASSERT(appSKey != NULL)
    
    self->devAddr = devAddr;
    (void)memcpy(self->nwkSKey, nwkSKey, sizeof(self->nwkSKey));
    (void)memcpy(self->appSKey, appSKey, sizeof(self->appSKey));    
}

bool LoraMAC_addChannel(struct lora_mac *self, uint8_t chIndex, uint32_t freq)
{
    LORA_ASSERT(self != NULL)

    return ChannelList_add(self->channels, chIndex, freq);
}

void LoraMAC_removeChannel(struct lora_mac *self, uint8_t chIndex)
{
    LORA_ASSERT(self != NULL)
    
    ChannelList_remove(self->channels, chIndex);
}

bool LoraMAC_maskChannel(struct lora_mac *self, uint8_t chIndex)
{
    LORA_ASSERT(self != NULL)
    
    return ChannelList_mask(self->channels, chIndex);
}

void LoraMAC_unmaskChannel(struct lora_mac *self, uint8_t chIndex)
{
    LORA_ASSERT(self != NULL)
    
    ChannelList_unmask(self->channels, chIndex);
}

bool LoraMac_setRateAndPower(struct lora_mac *self, uint8_t rate, uint8_t power)
{
    LORA_ASSERT(self != NULL)
    
    return ChannelList_setRateAndPower(self->channels, rate, power);
}



bool LoraMAC_send(struct lora_mac *self, bool confirmed, uint8_t port, const void *data, uint8_t len, void *receiver, txCompleteCB cb)
{
    LORA_ASSERT(self != NULL)
    LORA_ASSERT((len == 0) || (data != NULL))
    
    bool retval = false;
    uint16_t bufferMax = LoraFrame_getPhyPayloadSize(0, len);
    //uint32_t timeNow = 0U;      //fixme
    
    struct lora_channel_setting settings;
    
    /* stack must be idle (i.e. not sending) */
    if(self->state == IDLE){
    
        /* user port must be greater than 0 */
        if(port > 0U){

            if(ChannelList_upstreamSetting(self->channels, &settings)){
                
                if(settings.maxPayload >= bufferMax){ 

                    struct lora_frame f = {

                        .type = FRAME_TYPE_DATA_UNCONFIRMED_UP,
                        .devAddr = self->devAddr,
                        .counter = upCount(self),
                        .ack = false,
                        .adr = false,
                        .adrAckReq = false,
                        .pending = false,
                        .opts = NULL,
                        .optsLen = 0U,
                        .port = port,
                        .data = ((len > 0U) ? (const uint8_t *)data : NULL),
                        .dataLen = len
                    };

                    self->bufferLen = LoraFrame_encode(self->appSKey, &f, self->buffer, sizeof(self->buffer));
                    
                    //wait = ChanneList_waitTime(self->channels, timeNow);
                    
                    //check if beacon interval
                    
                    (void)Event_onTimeout(self->events, ChanneList_waitTime(self->channels, timeNow), self, tx);
                    
                    self->state = WAIT_TX;
                    
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
            
            LORA_ERROR("port 0 is reserved for MAC commands")
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

bool LoraMAC_requestJoin(struct lora_mac *self, void *receiver, joinConfirmation cb)
{
    uint8_t buffer[8U+8U+2U];
    
    if(self->state == IDLE){
        
        (void)getAppEUI(buffer, 8U);
        (void)getDevEUI(&buffer[8], 8U);
        (void)memcpy(&buffer[16], self->devNonce, sizeof(self->devNonce));
        
        struct lora_frame f = {

            .type = FRAME_TYPE_JOIN_REQ,
            .devAddr = 0U,
            .counter = upCount(self),
            .ack = false,
            .adr = false,
            .adrAckReq = false,
            .pending = false,
            .opts = NULL,
            .optsLen = 0U,
            .port = port,
            .data = ((len > 0U) ? (const uint8_t *)data : NULL),
            .dataLen = len
        };

        self->bufferLen = LoraFrame_encode(self->appKey, &f, self->buffer, sizeof(self->buffer));
        
        //wait = ChanneList_waitTime(self->channels, timeNow);
        
        //check if beacon interval
        
        (void)Event_onTimeout(self->events, 0, self, tx);
        
        self->state = WAIT_TX;
        
        retval = true;        
    }
}

/* static functions ***************************************************/

static uint32_t upCount(struct lora_mac *self)
{
    self->upCounter++;
    return self->upCounter;
}

static void tx(void *receiver, uint32_t time)
{
    struct lora_mac *self = (struct lora_mac *)receiver;    
    struct lora_channel_setting settings;
    
    if(ChannelList_upstreamSetting(self->channels, &settings)){
    
        if(LoraRadio_setParameters(self->radio, settings.freq, settings.bw, settings.sf)){
        
            self->state = TX;
            
            self->txComplete = Event_onInput(self->events, time + EVENT_TX_COMPLETE, self, txComplete);
            
            self->resetRadio = Event_onTimeout(self->events, time + RX_WDT_INTERVAL, self, resetRadio);
            
            LoraRadio_transmit(self->radio, self->buffer, self->bufferLen);
        }
        else{
            
            //handle parameter rejection
        }
    }
    else{
        
        // abort somehow
    }
}

static void txComplete(void *receiver, uint32_t time)
{
    struct lora_mac *self = (struct lora_mac *)receiver;    
    uint32_t interval;
    
    if(time < self->rx1_interval){
        
        self->state = WAIT_RX1;
    
        interval = self->rx1_interval - time;
    
        (void)Event_onTimeout(self->events, interval, self, rx1Start);    
    }    
}

static void rx1Start(void *receiver, uint32_t time)
{
    struct lora_mac *self = (struct lora_mac *)receiver;    
    struct lora_channel_setting settings;
    //uint32_t interval;
    
    if(ChannelList_rx1Setting(self->channels, &settings)){
    
        if(LoraRadio_setParameters(self->radio, settings.freq, settings.bw, settings.sf)){
    
            if(time < self->rx2_interval){
        
                self->state = RX1;
                
                //interval = INTERVAL_RX2 - time;
                
                self->rxReady = Event_onInput(self->events, EVENT_RX_READY, self, rx1Ready);
                self->rxTimeout = Event_onInput(self->events, EVENT_RX_TIMEOUT, self, rx1Timeout);
            
                LoraRadio_receive(self->radio);                                
            }
        }
        else{
            
            //handle parameter rejection
        }
    }
    else{
        
        // abort somehow
    }
}

static void rx1Ready(void *receiver, uint32_t time)
{
    struct lora_mac *self = (struct lora_mac *)receiver;    
    Event_cancel(self->events, &self->rxTimeout);    
    collect(self);            
    rx1Finish(self, time);
}

static void rx1Timeout(void *receiver, uint32_t time)
{
    struct lora_mac *self = (struct lora_mac *)receiver;    
    Event_cancel(self->events, &self->rxReady);    
    rx1Finish(self, time);
}

static void rx1Finish(struct lora_mac *self, uint32_t time)
{
    uint32_t timeNow = 0U;  //fixme
    
    ChannelList_registerTransmission(self->channels, timeNow, self->bufferLen);
    self->state = WAIT_RX2;    
    
    
    (void)Event_onTimeout(self->events, self->rx2_interval, self, rx2Start);
}

static void rx2Start(void *receiver, uint32_t time)
{
    struct lora_mac *self = (struct lora_mac *)receiver;    
    struct lora_channel_setting settings;
    
    if(ChannelList_rx2Setting(self->channels, &settings)){
    
        if(LoraRadio_setParameters(self->radio, settings.freq, settings.bw, settings.sf)){
        
            self->state = RX2;    
            
            self->rxReady = Event_onInput(self->events, EVENT_RX_READY, self, rx2Ready);
            self->rxTimeout = Event_onInput(self->events, EVENT_RX_TIMEOUT, self, rx2Timeout);
            
            LoraRadio_receive(self->radio);                                
        }
        else{
            
            //abort somehow
        }
    }
    else{
        
        //abort somehow
    }
}

static void rx2Ready(void *receiver, uint32_t time)
{
    struct lora_mac *self = (struct lora_mac *)receiver;    
    Event_cancel(self->events, &self->rxTimeout);    
    collect(self);            
    rx2Finish(self, time);
}

static void rx2Timeout(void *receiver, uint32_t time)
{
    struct lora_mac *self = (struct lora_mac *)receiver;    
    Event_cancel(self->events, &self->rxReady);    
    rx2Finish(self, time);
}

static void rx2Finish(struct lora_mac *self, uint32_t time)
{
    Event_cancel(self->events, &self->resetRadio);
    
    if(self->confirmed && (self->txCount < MAX_RETRIES)){
                    
        self->txCount++;
        self->state = WAIT_TX;
        (void)Event_onTimeout(self->events, ChannelList_waitTime(self->channels, getTime()), self, tx));
    }
    else if(!self->confirmed && (self->txCount < self->nbTrans)){
            
        self->txCount++;
        self->state = WAIT_TX;
        (void)Event_onTimeout(self->events, ChannelList_waitTime(self->channels, getTime()), self, tx));
    } 
    else{
        
        
        self->state = IDLE;
    }    
}

static void resetRadio(void *receiver, uint32_t time)
{
    struct lora_mac *self = (struct lora_mac *)receiver;    
    
    Event_cancel(self->events, &self->rxReady);
    self->rxReady = NULL;
    
    Event_cancel(self->events, &self->rxTimeout);
    self->rxTimeout = NULL;
    
    Event_cancel(self->events, &self->txComplete);
    self->txComplete = NULL;    
    
    //reset radio here?
    
    self->state = ERROR;
}
    
static void collect(struct lora_mac *self)
{
    struct lora_frame result;
    
    self->bufferLen = LoraRadio_collect(self->radio, self->buffer, sizeof(self->buffer));        
    
    if(LoraFrame_decode(self->nwkSKey, self->appSKey, self->buffer, self->bufferLen, &result)){
        
        switch(result.type){
        case FRAME_TYPE_JOIN_ACCEPT:
        
            if(self->joinPending){
                
                if(result.dataLen >= 12U){
                    
                    uint8_t appNonce[3];
                    uint8_t netID[3];
                    uint8_t devAddr[4];
                    uint8_t dlSettings;
                    uint8_t rxDelay;
                    uint8_t hdr;
                    
                    (void)memcpy(appNonce, &in[0], sizeof(appNonce));
                    pos += sizeof(appNonce);
                    (void)memcpy(netID, &in[pos], sizeof(netID));
                    pos += sizeof(netID);
                    (void)memcpy(devAddr, &in[pos], sizeof(devAddr));
                    pos += sizeof(devAddr);
                    dlSettings = in[pos];
                    pos += sizeof(dlSettings);
                    (void)memcpy(rxDelay, &in[pos], sizeof(rxDelay));
                    pos += sizeof(rxDelay);
                    
                    struct lora_aes_ctx aes;
                    struct lora_cmac_ctx aes;
                    
                    LoraAES_init(&aes, self->appKey);
                    
                    hdr = 1U;
                    LoraCMAC_init(&cmac, &aes);
                    LoraCMAC_update(&cmac, &hdr, sizeof(hdr));
                    LoraCMAC_update(&cmac, appNonce, sizeof(appNonce));
                    LoraCMAC_update(&cmac, netID, sizeof(netID));
                    LoraCMAC_update(&cmac, self->devNonce, sizeof(self->devNonce));
                    
                    LoraCMAC_finish(&cmac, self->nwkSKey, sizeof(self->nwkSKey));
                    
                    hdr = 2U;
                    LoraCMAC_init(&cmac, &aes);
                    LoraCMAC_update(&cmac, &hdr, sizeof(hdr));
                    LoraCMAC_update(&cmac, appNonce, sizeof(appNonce));
                    LoraCMAC_update(&cmac, netID, sizeof(netID));
                    LoraCMAC_update(&cmac, self->devNonce, sizeof(self->devNonce));
                    
                    LoraCMAC_finish(&cmac, self->appSKey, sizeof(self->nwkSKey));                    
                    
                    self->personalised = true;
                    self->joined = true;
                }
            }
            break
        
        case FRAME_TYPE_DATA_UNCONFIRMED_DOWN:
        case FRAME_TYPE_DATA_CONFIRMED_DOWN:
            
            if(self->personalised){
            
                if(self->devAddr == result.devAddr){
                
                    //check counter
                
                    if(result.optsLen > 0U){
                        
                        //process mac layer
                        //LoraMAC_processCommands(self, result.opts, result.optsLen);
                    }
                    
                    if(result.port == 0U){
                                    
                        //LoraMAC_processCommands(self, result.data, result.dataLen);
                    }
                    else{
                        
                        if(self->rxCompleteHandler){
                            
                            self->rxCompleteHandler(self->rxCompleteReceiver, self, result.port, result.data, result.dataLen);
                        }
                    }
                }
            }
            break;
        
        default:
            break;
        }
    }
}

#if 0
static void beaconStart(void *receiver, uint32_t time)
{
    LORA_ASSERT(self != NULL)
    LORA_ASSERT(self->state == IDLE)
    
    struct lora_mac *self = (struct lora_mac *)receiver;    
    struct lora_channel_setting settings;
    
    if(ChannelList_beaconSetting(self->channels, &settings)){
    
        if(LoraRadio_setParameters(self->radio, settings.freq, settings.bw, settings.sf)){
       
            self->state = BEACON_RX;
            
            self->rxReady = Event_onInput(self->events, EVENT_RX_READY, self, beaconReady);
            self->rxTimeout = Event_onInput(self->events, EVENT_RX_TIMEOUT, self, beaconTimeout);
            
            self->resetRadio = Event_onTimeout(self->events, RX_WDT_INTERVAL, self, resetRadio);
            
            LoraRadio_receive(self->radio);                                
        }
        else{
            
            //handle parameter rejection
        }
    }
    else{
        
        // abort somehow
    }
}

static void beaconTimeout(void *receiver, uint32_t time)
{
    LORA_ASSERT(self != NULL)
    LORA_ASSERT(self->state == BEACON_RX)
    
    struct lora_mac *self = (struct lora_mac *)receiver;    
    Event_cancel(self->events, &self->rxReady);    
    beaconFinish(self, time);
}

static void beaconReady(void *receiver, uint32_t time)
{   
    LORA_ASSERT(self != NULL)
    LORA_ASSERT(self->state == BEACON_RX)
    
    struct lora_mac *self = (struct lora_mac *)receiver;    
    Event_cancel(self->events, &self->rxTimeout);    
    collect(self);            
    beaconFinish(self, time); 
}

static void beaconFinish(struct lora_mac *self, uint32_t time)
{   
    Event_cancel(self->events, &self->resetRadio);
    self->state = IDLE;   
}

#endif

static void radioEvent(void *receiver, enum lora_radio_event event, uint32_t time)
{
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
