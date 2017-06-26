#include "lora_mac.h"
#include "lora_channel_list.h"
#include "lora_event.h"
#include "lora_frame.h"
#include "lora_debug.h"

#include <string.h>

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

static void beaconStart(void *receiver, uint32_t time);
static void beaconReady(void *receiver, uint32_t time);
static void beaconTimeout(void *receiver, uint32_t time);
static void beaconFinish(struct lora_mac *self, uint32_t time);

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
    
    struct region_defaults defaults = LoraRegion_getDefaultSettings(ChannelList_region(self->channels));
    
    
    
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

void LoraMAC_eventTXComplete(struct lora_mac *self, uint32_t time)
{
    Event_receive(self->events, EVENT_TX_COMPLETE, time);
}

void LoraMAC_eventRXReady(struct lora_mac *self, uint32_t time)
{
    Event_receive(self->events, EVENT_RX_READY, time);
}

void LoraMAC_eventRXTimeout(struct lora_mac *self, uint32_t time)
{
    Event_receive(self->events, EVENT_RX_TIMEOUT, time);
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
                    
                    (void)Event_onTimeout(self->events, 0, self, tx);
                    
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
            
            self->resetRadio = Event_onTimeout(self->events, time + DEVICE_WDT_INTERVAL, self, resetRadio);
            
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
    
    if(time < INTERVAL_RX1){
        
        self->state = WAIT_RX1;
    
        interval = INTERVAL_RX1 - time;
    
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
    
            if(time < INTERVAL_RX1){
        
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
    
    ChanneList_registerTransmission(self->channels, timeNow, self->bufferLen);
    self->state = WAIT_RX2;    
    
    LoraRegion_getDefaultSettings(ChannelList_region(self->channels), 
    
    (void)Event_onTimeout(self->events, WAIT_RX2_INTERVAL, self, rx2Start);
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
    
    if(!self->confirmed && (self->txCount < self->nbTrans)){
        
        self->txCount++;
        //do resend
    } 
    
    //check redundancy settings
    //check confirm settings
    self->state = IDLE;
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
        
        //check counter
        //check address?
        
        if(result.optsLen > 0U){
            
            //process mac layer
        }
        
        if(self->rxCompleteHandler){
            
            self->rxCompleteHandler(self->rxCompleteReceiver, self, result.port, result.data, result.dataLen);
        }
    }
}

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
            
            self->resetRadio = Event_onTimeout(self->events, DEVICE_WDT_INTERVAL, self, resetRadio);
            
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

