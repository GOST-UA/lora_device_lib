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
#include "lora_frame.h"
#include "lora_debug.h"
#include "lora_aes.h"
#include "lora_cmac.h"
#include "lora_system.h"
#include "lora_mac_commands.h"

#include <string.h>

/* types **************************************************************/


/* static function prototypes *****************************************/

static void tx(void *receiver, uint64_t time);
static void txComplete(void *receiver, uint64_t time);

static void rxStart(void *receiver, uint64_t time);
static void rxReady(void *receiver, uint64_t time);
static void rxTimeout(void *receiver, uint64_t time);
static void rxFinish(struct lora_mac *self);

//static void resetRadio(void *receiver, uint64_t time);
static void collect(struct lora_mac *self);

static void abandonSequence(struct lora_mac *self);

static void handleCommands(void *receiver, const struct lora_downstream_cmd *cmd);
static void processCommands(struct lora_mac *self, const uint8_t *data, uint8_t len);

static bool selectChannel(struct lora_mac *self, uint64_t timeNow, uint8_t rate, uint8_t *chIndex, uint32_t *freq);
static void registerTime(struct lora_mac *self, uint32_t freq, uint64_t timeNow, uint32_t airTime);
static void addDefaultChannel(void *receiver, uint8_t chIndex, uint32_t freq, uint8_t minRate, uint8_t maxRate);
static bool getChannel(struct lora_mac *self, uint8_t chIndex, uint32_t *freq, uint8_t *minRate, uint8_t *maxRate);
static bool isAvailable(struct lora_mac *self, uint8_t chIndex, uint64_t timeNow, uint8_t rate);

static uint64_t timeBase(uint8_t value);

//static uint64_t timeNextAvailable(struct lora_mac *self, uint64_t timeNow, uint8_t rate);

//static void restoreDefaults(struct lora_mac *self);

/* functions **********************************************************/

void MAC_init(struct lora_mac *self, void *system, enum lora_region_id region, struct lora_radio *radio)
{
    LORA_PEDANTIC(self != NULL)
    LORA_PEDANTIC(radio != NULL)

    (void)memset(self, 0, sizeof(*self));
    
    self->system = system;
    self->radio = radio;    
    self->region = Region_getRegion(region);
    
    Event_init(&self->events);
    
    Radio_setEventHandler(self->radio, self, MAC_radioEvent);

    //Region_getDefaultChannels(self->region, self, addDefaultChannel);    
}

bool MAC_send(struct lora_mac *self, bool confirmed, uint8_t port, const void *data, uint8_t len)
{
    LORA_PEDANTIC(self != NULL)
    LORA_PEDANTIC((len == 0) || (data != NULL))
    
    bool retval = false;
    
    struct lora_frame_data f;
    uint8_t key[16U];
    uint64_t timeNow = System_getTime();
    
    if(self->status.personalized || self->status.joined){
    
        /* stack must be idle (i.e. not sending) */
        if(self->state == IDLE){
        
            if((port > 0U) && (port <= 223U)){
                
                if(len <= Region_getPayload(self->region, System_getTXRate(self->system))){

                    if(selectChannel(self, timeNow, System_getTXRate(self->system), &self->tx.chIndex, &self->tx.freq)){
                
                        f.devAddr = System_getDevAddr(self->system);
                        f.counter = System_incrementUp(self->system);
                        f.ack = false;
                        f.adr = false;
                        f.adrAckReq = false;
                        f.pending = false;
                        f.opts = NULL;
                        f.optsLen = 0U;
                        f.port = port;
                        f.data = ((len > 0U) ? (const uint8_t *)data : NULL);
                        f.dataLen = len;
                        
                        if(port == 0U){
                            
                            System_getNwkSKey(self->system, key);
                        }
                        else{
                            
                            System_getAppSKey(self->system, key);
                        }
                        
                        self->bufferLen = Frame_putData(FRAME_TYPE_DATA_UNCONFIRMED_UP, key, &f, self->buffer, sizeof(self->buffer));
                        
                        (void)Event_onTimeout(&self->events, 0U, self, tx);
                        
                        self->state = WAIT_TX;
                        
                        self->status.confirmPending = true;
                        self->status.confirmed = false;
                        
                        retval = true;                    
                    }
                    else{
                        
                        LORA_ERROR("no channel available")
                    }                    
                }
                else{
                    
                    LORA_ERROR("payload too large")
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
        
        LORA_ERROR("stack must be personalized or joined before sending data")
    }
    
    return retval;
}

bool MAC_join(struct lora_mac *self)
{
    bool retval = false;
    
    uint64_t timeNow = System_getTime();
    
    if(!self->status.personalized){
    
        if(self->state == IDLE){

            if(selectChannel(self, timeNow, System_getTXRate(self->system), &self->tx.chIndex, &self->tx.freq)){

                struct lora_frame_join_request f;      
                uint8_t appKey[16U];      
                
                System_getAppKey(self->system, appKey);
                
                System_getAppEUI(self->system, f.appEUI);
                System_getDevEUI(self->system, f.devEUI);            
                
                self->devNonce = System_rand();
                self->devNonce <<= 8;
                self->devNonce |= System_rand();
                
                f.devNonce = self->devNonce;

                self->bufferLen = Frame_putJoinRequest(appKey, &f, self->buffer, sizeof(self->buffer));
                
                (void)Event_onTimeout(&self->events, 0U, self, tx);
                
                self->state = JOIN_WAIT_TX;
                self->status.joinPending = true;
                
                retval = true;        
            }
            else{
                
                LORA_ERROR("no channel available")
            }
        }
        else{
            
            LORA_ERROR("ldl must be in IDLE state to perform join")
        }
    }
    else{
        
        LORA_ERROR("stack has been personalized - join is not possible")
    }   
    
    return retval;
}

void MAC_setResponseHandler(struct lora_mac *self, void *receiver, lora_mac_response_fn cb)
{
    self->responseHandler = cb;
    self->responseReceiver = receiver;
}

bool MAC_personalize(struct lora_mac *self, uint32_t devAddr, const void *nwkSKey, const void *appSKey)
{
    LORA_PEDANTIC(self != NULL)
    LORA_PEDANTIC(nwkSKey != NULL)
    LORA_PEDANTIC(appSKey != NULL)
    
    bool retval = false;
    
    if(self->state == IDLE){
    
        System_setDevAddr(self->system, devAddr);
        System_setNwkSKey(self->system, nwkSKey);
        System_setAppSKey(self->system, appSKey);
     
        self->status.personalized = true;
     
        retval = true;
    }
    else{
                
        LORA_ERROR("cannot personalize while not idle")
    }
    
    return retval;
}

void MAC_radioEvent(void *receiver, enum lora_radio_event event, uint64_t time)
{
    LORA_PEDANTIC(receiver != NULL)
    
    struct lora_mac *self = (struct lora_mac *)receiver;
    
    switch(event){
    case LORA_RADIO_TX_COMPLETE:
        Event_receive(&self->events, EVENT_TX_COMPLETE, time);
        break;
    case LORA_RADIO_RX_READY:
        Event_receive(&self->events, EVENT_RX_READY, time);
        break;
    case LORA_RADIO_RX_TIMEOUT:
        Event_receive(&self->events, EVENT_RX_TIMEOUT, time);
        break;
    default:
        break;
    }
}

uint32_t MAC_calculateOnAirTime(enum lora_signal_bandwidth bw, enum lora_spreading_factor sf, uint8_t payloadLen)
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
        uint32_t Tpreamble = (Ts * 12U) +  (Ts / 4U);   //preamble (us)

        uint32_t numerator = (8U * (uint32_t)payloadLen) - (4U * (uint32_t)sf) + 28U + ( crc ? 16U : 0U ) - ( (header) ? 20U : 0U );
        uint32_t denom = 4U * ((uint32_t)sf - ( lowDataRateOptimize ? 2U : 0U ));

        uint32_t Npayload = 8U + ((numerator / denom) + (((numerator % denom) != 0) ? 1U : 0U)) * ((uint32_t)CR_5 + 4U);

        uint32_t Tpayload = Npayload * Ts;

        Tpacket = Tpreamble + Tpayload;
    }

    return Tpacket / 10U;
}

void MAC_tick(struct lora_mac *self)
{
    Event_tick(&self->events);
}

uint64_t MAC_intervalUntilNext(struct lora_mac *self)
{
    return Event_intervalUntilNext(&self->events);
}

bool MAC_setRate(struct lora_mac *self, uint8_t rate)
{
    bool retval = false;
    
    if(rate <= 0xfU){
        
        System_setTXRate(self->system, rate);
        retval = true;
    }
    
    return retval;
}

bool MAC_setPower(struct lora_mac *self, uint8_t power)
{
    bool retval = false;
    
    if(power <= 0xfU){
    
        System_setTXPower(self->system, power);    
        retval = true;
    }
    
    return retval;    
}

/* static functions ***************************************************/

void MAC_restoreDefaults(struct lora_mac *self)
{
    LORA_PEDANTIC(self != NULL)
    
    struct lora_region_default defaults;
    
    Region_getDefaultChannels(self->region, self->system, addDefaultChannel);    
    
    Region_getDefaultSettings(self->region, &defaults);
        
    System_setRX1DROffset(self->system, defaults.rx1_offset);
    System_setRX1Delay(self->system, defaults.rx1_delay);
    
    System_setRX2DataRate(self->system, defaults.rx2_rate);
    System_setRX2Freq(self->system, defaults.rx2_freq);
    
    System_setMaxDutyCycle(self->system, 1U);        
}

static void tx(void *receiver, uint64_t time)
{
    LORA_PEDANTIC(receiver != NULL)
    
    struct lora_mac *self = (struct lora_mac *)receiver;        
    struct lora_data_rate rate_setting;
    
    if(Region_getRate(self->region, System_getTXRate(self->system), &rate_setting)){
    
        struct lora_radio_tx_setting radio_setting = {
            
            .freq = self->tx.freq,
            .bw = rate_setting.bw,
            .sf = rate_setting.sf,
            .cr = CR_5,
            .power = System_getTXPower(self->system)         
        };
    
        LORA_PEDANTIC((self->state == JOIN_WAIT_TX) || (self->state == WAIT_TX))
    
        if(Radio_transmit(self->radio, &radio_setting, self->buffer, self->bufferLen)){

            registerTime(self, self->tx.freq, System_getTime(), MAC_calculateOnAirTime(radio_setting.bw, radio_setting.sf, self->bufferLen));
            
            self->state = (self->state == WAIT_TX) ? TX : JOIN_TX;
                
            self->txComplete = Event_onInput(&self->events, EVENT_TX_COMPLETE, self, txComplete);
        
            self->resetRadio = NULL;    //todo: add a watchdog to catch situation where IO lines don't work as expected    
        }
        else{
            
            LORA_INFO("radio rejected parameters")            
            abandonSequence(self);
        }
    }
    else{
        
        LORA_INFO("invalid rate")            
        abandonSequence(self);
    }
}

static void txComplete(void *receiver, uint64_t time)
{
    LORA_PEDANTIC(receiver != NULL)
    
    struct lora_mac *self = (struct lora_mac *)receiver;            
    uint64_t rx1Time;
        
    LORA_PEDANTIC((self->state == JOIN_TX) || (self->state == TX))
    
    Radio_sleep(self->radio);
    
    self->txCompleteTime = time;
        
    rx1Time = self->txCompleteTime + ((self->state == TX) ? timeBase(System_getRX1Delay(self->system)) : timeBase(Region_getJA1Delay(self->region)));
    
    self->state = (self->state == TX) ? WAIT_RX1 : JOIN_WAIT_RX1;                

    (void)Event_onTimeout(&self->events, rx1Time, self, rxStart);
}

static void rxStart(void *receiver, uint64_t time)
{
    LORA_PEDANTIC(receiver != NULL)
    
    struct lora_mac *self = (struct lora_mac *)receiver;    
    struct lora_radio_rx_setting radio_setting;
    
    uint8_t rate;
    uint32_t freq;
    struct lora_data_rate rate_setting;
    
    uint64_t timeNow = System_getTime();
    uint64_t rxTime = time;   // fixme: need to account for fudge factor (which we don't currently add)
        
    LORA_PEDANTIC((self->state == WAIT_RX1) || (self->state == WAIT_RX2) || (self->state == JOIN_WAIT_RX1) || (self->state == JOIN_WAIT_RX2))
    LORA_PEDANTIC(time <= timeNow)
   
    if(timeNow <= rxTime){
    
        // if we are too early we can block (todo: or something else)
        if(timeNow < rxTime){
            
            System_usleep(rxTime - timeNow);
        }
        
        switch(self->state){
        default:
        case JOIN_WAIT_RX1:
        case WAIT_RX1:
        
            (void)Region_getRX1DataRate(self->region, System_getTXRate(self->system), System_getRX1DROffset(self->system), &rate);
            (void)Region_getRX1Freq(self->region, self->tx.freq, &freq);
            
            self->state = (self->state == WAIT_RX1) ? RX1 : JOIN_RX1;                            
            break;
        
        case JOIN_WAIT_RX2:    
        case WAIT_RX2:    
        
            rate = System_getRX2DataRate(self->system);    
            freq = System_getRX2Freq(self->system);
        
            self->state = (self->state == WAIT_RX2) ? RX2 : JOIN_RX2;                
            break;
        }
        
        (void)Region_getRate(self->region, rate, &rate_setting);
        
        // fixme: where is the transceiver timeout setting?
        
        radio_setting.freq = freq;
        radio_setting.bw = rate_setting.bw;
        radio_setting.sf = rate_setting.sf;
        radio_setting.cr = CR_5;
        radio_setting.preamble = 8U;                
            
        if(Radio_receive(self->radio, &radio_setting)){
             
            self->rxReady = Event_onInput(&self->events, EVENT_RX_READY, self, rxReady);
            self->rxTimeout = Event_onInput(&self->events, EVENT_RX_TIMEOUT, self, rxTimeout);                            
        }
        else{
            
            LORA_INFO("could not apply radio settings")
            abandonSequence(self);
        }         
    }
    else{
        
        switch(self->state){
        default:
        case JOIN_WAIT_RX1:
            LORA_INFO("missed JOIN RX1 deadline by %" PRIu64, timeNow - rxTime)
            self->state = JOIN_RX1;
            break;
            
        case WAIT_RX1:
            LORA_INFO("missed RX1 deadline %" PRIu64, timeNow - rxTime)
            self->state = RX1;
            break;
            
        case JOIN_WAIT_RX2:    
            LORA_INFO("missed JOIN RX2 deadline %" PRIu64, timeNow - rxTime)
            self->state = JOIN_RX2;
            break;
        
        case WAIT_RX2:    
            LORA_INFO("missed RX2 deadline %" PRIu64, timeNow - rxTime)
            self->state = RX2;
            break;
        }
        
        rxFinish(self);
    }
}

static void rxReady(void *receiver, uint64_t time)
{
    LORA_PEDANTIC(receiver != NULL)
        
    struct lora_mac *self = (struct lora_mac *)receiver;    
    Event_cancel(&self->events, &self->rxTimeout);    
    
    LORA_ASSERT((self->state == RX1) || (self->state == RX2) || (self->state == JOIN_RX1) || (self->state == JOIN_RX2))
    
    collect(self);            
    rxFinish(self);
}

static void rxTimeout(void *receiver, uint64_t time)
{
    LORA_PEDANTIC(receiver != NULL)
    
    struct lora_mac *self = (struct lora_mac *)receiver;    
    Event_cancel(&self->events, &self->rxReady);    
    
    LORA_ASSERT((self->state == RX1) || (self->state == RX2) || (self->state == JOIN_RX1) || (self->state == JOIN_RX2))
    
    rxFinish(self);
}

static void rxFinish(struct lora_mac *self)
{    
    LORA_PEDANTIC(self != NULL)
    
    Radio_sleep(self->radio);
    
    switch(self->state){
    default:
    case RX1:        
    
        self->state = WAIT_RX2;    
        (void)Event_onTimeout(&self->events, self->txCompleteTime + timeBase(System_getRX1Delay(self->system) + 1U), self, rxStart);
        break;
    
    case JOIN_RX1:        
    
        self->state = JOIN_WAIT_RX2;    
        (void)Event_onTimeout(&self->events, self->txCompleteTime + timeBase(Region_getJA1Delay(self->region) + 1U), self, rxStart);
        break;
        
    case RX2:        
        
        if(self->responseHandler != NULL){

            self->responseHandler(self->responseReceiver, LORA_MAC_DATA_COMPLETE, NULL);
        }
        self->state = IDLE;            
        break;
    
    case JOIN_RX2:   
        
        if(self->responseHandler != NULL){
            
            self->responseHandler(self->responseReceiver, (self->status.joined) ? LORA_MAC_JOIN_SUCCESS : LORA_MAC_JOIN_TIMEOUT, NULL);
        }
        self->state = IDLE;            
        break;
    }        
}

#if 0
static void resetRadio(void *receiver, uint64_t time)
{
    LORA_PEDANTIC(receiver != NULL)
    
    struct lora_mac *self = (struct lora_mac *)receiver;    
    uint32_t delay;
    
    LORA_INFO("radio has been reset")
    
    Event_cancel(&self->events, &self->rxReady);
    self->rxReady = NULL;
    
    Event_cancel(&self->events, &self->rxTimeout);
    self->rxTimeout = NULL;
    
    Event_cancel(&self->events, &self->txComplete);
    self->txComplete = NULL;    
    
    delay = Radio_resetHardware(self->radio);
    
    abandonSequence(self);
}
#endif
    
static void collect(struct lora_mac *self)
{
    struct lora_frame result;
    
    uint8_t appKey[16U];
    uint8_t appSKey[16U];
    uint8_t nwkSKey[16U];
        
    System_getAppKey(self->system, appKey);
    System_getAppSKey(self->system, appSKey);
    System_getNwkSKey(self->system, nwkSKey);
    
    self->bufferLen = Radio_collect(self->radio, self->buffer, sizeof(self->buffer));        
    
    if(Frame_decode(appKey, nwkSKey, appSKey, self->buffer, self->bufferLen, &result)){
        
        switch(result.type){
        case FRAME_TYPE_JOIN_ACCEPT:
        
            if(self->status.joinPending){
                
                self->status.joinPending = false;
                self->status.joined = true;                

                System_resetUp(self->system);
                System_resetDown(self->system);
                
                MAC_restoreDefaults(self);
                
                System_setRX1DROffset(self->system, result.fields.joinAccept.rx1DataRateOffset);
                System_setRX2DataRate(self->system, result.fields.joinAccept.rx2DataRate);

                if(result.fields.joinAccept.cfListPresent){
                    
                    if(Region_isDynamic(self->region)){
   
                        size_t i;
                    
                        for(i=0U; i < sizeof(result.fields.joinAccept.cfList)/sizeof(*result.fields.joinAccept.cfList); i++){
                            
                            uint8_t chIndex = 4U + i;
                            
                            //Region_validateFreq(self, chIndex, result.fields.joinAccept.cflist[i]);
                            
                            (void)System_setChannel(self->system, chIndex, result.fields.joinAccept.cfList[i], 0U, 5U);
                        }                           
                    }                    
                }   
                
                struct lora_aes_ctx ctx;
                LoraAES_init(&ctx, appKey);
                
                (void)memset(nwkSKey, 0U, sizeof(nwkSKey));
                
                nwkSKey[0] = 1U;                
                nwkSKey[1] = result.fields.joinAccept.appNonce;
                nwkSKey[2] = result.fields.joinAccept.appNonce >> 8;
                nwkSKey[3] = result.fields.joinAccept.appNonce >> 16;
                nwkSKey[4] = result.fields.joinAccept.netID;
                nwkSKey[5] = result.fields.joinAccept.netID >> 8;
                nwkSKey[6] = result.fields.joinAccept.netID >> 16;
                nwkSKey[7] = self->devNonce;
                nwkSKey[8] = self->devNonce >> 8;
                
                (void)memcpy(appSKey, nwkSKey, sizeof(appSKey));
                
                appSKey[0] = 2U;                
                
                LoraAES_encrypt(&ctx, nwkSKey);
                LoraAES_encrypt(&ctx, appSKey);
                                
                System_setAppSKey(self->system, nwkSKey);                
                System_setAppSKey(self->system, appSKey);
            }
            else{
                
                LORA_INFO("ignoring a JOIN Accept we didn't ask for")
            }
            break;
        
        case FRAME_TYPE_DATA_UNCONFIRMED_DOWN:
        case FRAME_TYPE_DATA_CONFIRMED_DOWN:
            
            if(self->status.personalized || self->status.joined){
            
                if(System_getDevAddr(self->system) == result.fields.data.devAddr){
                
                    if(System_receiveDown(self->system, result.fields.data.counter, Region_getMaxFCNTGap(self->region))){
                    
                        processCommands(self, result.fields.data.opts, result.fields.data.optsLen);
                        
                        if(result.fields.data.data != NULL){
                
                            if(result.fields.data.port > 0U){
                                
                                if(self->responseHandler != NULL){
                                    
                                    union lora_mac_response_arg arg;
                                    
                                    arg.rx.data = result.fields.data.data;
                                    arg.rx.len = result.fields.data.dataLen;
                                    arg.rx.port = result.fields.data.port;
                                    
                                    self->responseHandler(self->responseReceiver, LORA_MAC_RX, &arg);
                                }
                            }
                            else{
                                                                
                                processCommands(self, result.fields.data.data, result.fields.data.dataLen);
                            }
                        }
                    }
                    else{
                        
                        LORA_INFO("discarding frame for counter sync")
                    }
                }
            }
            break;
        
        default:
            break;
        }
    }
}

static void handleCommands(void *receiver, const struct lora_downstream_cmd *cmd)
{
    struct lora_mac *self = (struct lora_mac *)receiver;
    
    switch(cmd->type){
    default:
    case LINK_CHECK:
    
        System_logLinkStatus(self->system, cmd->fields.linkCheckAns.margin, cmd->fields.linkCheckAns.gwCount);
        break;
        
    case LINK_ADR:                    
        //these need be processed as a transaction
        break;
    
    case DUTY_CYCLE:                
        System_setMaxDutyCycle(self->system, cmd->fields.dutyCycleReq.maxDutyCycle);
        break;
    
    case RX_PARAM_SETUP:
        
        break;
    
    case DEV_STATUS:
    {
        //ans.battery = System_getBatteryLevel(self->system);
        //ans.margin = 0U;
    }
        break;
        
    case NEW_CHANNEL:    
    
        if(Region_isDynamic(self->region)){
        
            struct lora_new_channel_ans ans;
        
            ans.dataRateRangeOK = Region_validateRate(self->region, cmd->fields.newChannelReq.chIndex, cmd->fields.newChannelReq.minDR, cmd->fields.newChannelReq.maxDR);        
            ans.channelFrequencyOK = Region_validateFreq(self->region, cmd->fields.newChannelReq.chIndex, cmd->fields.newChannelReq.freq);
            
            if(ans.dataRateRangeOK && ans.channelFrequencyOK){
                
                (void)System_setChannel(self->system, cmd->fields.newChannelReq.chIndex, cmd->fields.newChannelReq.freq, cmd->fields.newChannelReq.minDR, cmd->fields.newChannelReq.maxDR);                        
            }            
        }
        break;        
    case DL_CHANNEL:            
        break;
    
    case RX_TIMING_SETUP:    
    
        break;
    
    case TX_PARAM_SETUP:        
        break;
    }    
}

static void processCommands(struct lora_mac *self, const uint8_t *data, uint8_t len)
{
    (void)MAC_eachDownstreamCommand(self, data, len, handleCommands);
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

static void registerTime(struct lora_mac *self, uint32_t freq, uint64_t timeNow, uint32_t airTime)
{
    LORA_PEDANTIC(self != NULL)
    
    uint8_t band;
    
    if(Region_getBand(self->region, freq, &band)){
    
        self->bands[band] = timeNow + ( airTime * Region_getOffTimeFactor(self->region, band));
    }
}    

static void addDefaultChannel(void *receiver, uint8_t chIndex, uint32_t freq, uint8_t minRate, uint8_t maxRate)
{
    (void)System_setChannel(receiver, chIndex, freq, minRate, maxRate);
}

static bool selectChannel(struct lora_mac *self, uint64_t timeNow, uint8_t rate, uint8_t *chIndex, uint32_t *freq)
{
    bool retval = false;
    uint8_t i;    
    uint8_t available = 0U;
    uint8_t j = 0U;
    uint8_t minRate;
    uint8_t maxRate;
    
    for(i=0U; i < Region_numChannels(self->region); i++){
        
        if(isAvailable(self, i, timeNow, rate)){
        
            available++;            
        }            
    }
    
    if(available > 0U){
    
        uint8_t index;
    
        index = System_rand() % available;
        
        available = 0U;
        
        for(i=0U; i < Region_numChannels(self->region); i++){
        
            if(isAvailable(self, i, timeNow, rate)){
            
                if(index == j){
                    
                    if(getChannel(self, i, freq, &minRate, &maxRate)){
                        
                        *chIndex = i;
                        retval = true;
                        break;
                    }                        
                }
        
                j++;            
            }            
        }
    }

    return retval;
}

static bool isAvailable(struct lora_mac *self, uint8_t chIndex, uint64_t timeNow, uint8_t rate)
{
    bool retval = false;
    uint32_t freq;
    uint8_t minRate;    
    uint8_t maxRate;    
    uint8_t band;
    
    if(!System_channelIsMasked(self->system, chIndex)){
    
        if(getChannel(self, chIndex, &freq, &minRate, &maxRate)){
            
            if(rate >= minRate && rate <= maxRate){
            
                if(Region_getBand(self->region, freq, &band)){
                
                    if(timeNow >= self->bands[band]){
                    
                        retval = true;                    
                    }
                }
            }
        }
    }
    
    return retval;
}

#if 0
static uint64_t timeNextAvailable(struct lora_mac *self, uint64_t timeNow, uint8_t rate)
{
    uint8_t i;
    uint32_t freq;
    uint8_t minRate;
    uint8_t band;
    uint8_t maxRate;
    uint8_t nextBand = UINT8_MAX;
    
    for(i=0U; i < Region_numChannels(self->region); i++){
     
        if(!System_channelIsMasked(self->system, i)){
        
            if(getChannel(self, i, &freq, &minRate, &maxRate)){
                
                if(rate >= minRate && rate <= maxRate){
                
                    if(Region_getBand(self->region, freq, &band)){
                
                        if(nextBand == UINT8_MAX){
                            
                            nextBand = band;
                        }
                        else{
                        
                            if(nextBand != band){
                                
                                if(self->bands[band] < self->bands[nextBand]){
                                    
                                    nextBand = band;
                                }
                            }
                        }                                                    
                    }
                }
            }
        }
    }
    
    return (nextBand == UINT8_MAX) ? UINT64_MAX : self->bands[band];
}
#endif

static bool getChannel(struct lora_mac *self, uint8_t chIndex, uint32_t *freq, uint8_t *minRate, uint8_t *maxRate)
{
    bool retval = false;
    
    if(Region_isDynamic(self->region)){
        
        retval = System_getChannel(self->system, chIndex, freq, minRate, maxRate);                        
    }
    else{
        
        retval = Region_getChannel(self->region, chIndex, freq, minRate, maxRate);                        
    }
     
    return retval;
}


static uint64_t timeBase(uint8_t value)
{
    return ((uint64_t)value) * 100000U;
}
