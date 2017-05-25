
#include "lora_mac.h"
#include "lora_frame.h"

#include <string.h>

/* static function prototypes *****************************************/



/* functions **********************************************************/

void LoraMAC_init(struct lora_mac *self, struct lora_mac_init *param);
{
    LORA_ASSERT(self != NULL)
    LORA_ASSERT(param != NULL)

    (void)memset(self, 0, sizeof(*self));

    ChannelList_init(&self->channels, param->region);
    LoraRadio_init(&self->radio, param->board, param->radioType);
}

void LoraMAC_setSession(struct lora_mac *self, uint32_t devAddr, const uint8_t *nwkSKey, const uint8_t *appSKey)
{
    LORA_ASSERT(self != NULL)
    LORA_ASSERT(nwkSKey != NULL)
    LORA_ASSERT(appSKey != NULL)
    
    self->devAddr = devAddr;
    (void)memcpy(self->nwkSKey, nwkSKey, sizeof(self->nwkSKey));
    (void)memcpy(self->appSKey, appSKey, sizeof(self->appSKey));    
}

bool LoraMAC_addChannel(struct lora_mac *self, uint32_t freq)
{
    LORA_ASSERT(self != NULL)

    return ChannelList_add(&self->channels, freq);
}

void LoraMAC_removeChannel(struct lora_mac *self, uint32_t freq)
{
    LORA_ASSERT(self != NULL)
    
    ChannelList_remove(&self->channels, freq);
}

bool LoraMAC_maskChannel(struct lora_mac *self, uint32_t freq)
{
    LORA_ASSERT(self != NULL)
    
    ChannelList_mask(&self->channels, freq);
}

void LoraMAC_unmaskChannel(struct lora_mac *self, uint32_t freq)
{
    LORA_ASSERT(self != NULL)
    
    ChannelList_unmask(&self->channels, freq);
}

bool LoraMac_setRateAndPower(struct lora_mac *self, uint8_t rate, uint8_t power)
{
    LORA_ASSERT(self != NULL)
    
    return ChannelList_setRateAndPower(&self->channels, rate, power);
}

bool LoraMAC_unconfirmedUp(struct lora_mac *self, uint8_t port, const void *data, uint8_t len, txCompleteCB cb)
{
    LORA_ASSERT(self != NULL)
    
    bool retval = false;
    uint16_t bufferMax = LoraFrame_getPhyPayloadSize(0, len);
    uint8_t buffer[256U];
    uint8_t bufferLen;
    uint32_t timeNow = 0U;      //fixme
    
    if((len == 0U) || (port > 0U)){

        if(ChannelList_frequency(&self->channels) > 0U){

            if((uint16_t)ChanneList_maxPayload(&self->channels) >= bufferMax){

                if(ChanneList_waitTime(&self->channels, timeNow) == 0U){

                    struct lora_frame f = {

                        .type = FRAME_TYPE_DATA_UNCONFIRMED_UP,
                        .devAddr = devAddr,
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

                    bufferLen = LoraFrame_encode(appSKey, &f, buffer, bufferMax);

                    LORA_ASSERT(bufferLen == bufferMax)

                    if(LoraRadio_setParameters(&self->radio, ChannelList_frequency(&self->channels), ChannelList_bw(&self->channels), ChannelList_sf(&self->channels))){
                    
                        LoraRadio_transmit(&self->radio, buffer, bufferLen);
                        ChanneList_accountForTransmission(&self->channels, 0U, bufferLen);       //fixme

                        Scheduler_listenFor(&self->eventManager, EVENT_TX_COMPLETE, handler)

                        retval = true;
                    }
                }
            }
        }
    }

    return retval;
}

void LoraMAC_eventTXComplete(struct lora_mac *self)
{
    //LoraEventManager_schedule(&self->eventManager, handleTXComplete , 0);
}

void LoraMAC_eventRXComplete(struct lora_mac *self)
{
    //LoraEventManager_schedule(&self->eventManager, handleRXComplete , 0);
}

void LoraMAC_eventRXTimeout(struct lora_mac *self)
{
    //LoraEventManager_schedule(&self->eventManager, handleRXTimeout , 0);
}

/* static functions ***************************************************/

static void fsm(struct lora_mac *self, enum ev event)
{
    switch(self->state){
    default:
    case IDLE:
        break;
    case WAIT_TX_COMPLETE:

        switch(event){
        default:
        case TX_COMPLETE:
            break;
        }
        
        break;
    case WAIT_RX_EVENT:
        break;
    }
}

static void handleTXComplete(struct lora_mac *self)
{
}
static void handleRXComplete(struct lora_mac *self)
{
}
static void handleRXTimeout(struct lora_mac *self)
{
}

static uint32_t upCount(struct lora_mac *self)
{
    self->upCounter++;
    return self->upCounter;
}

