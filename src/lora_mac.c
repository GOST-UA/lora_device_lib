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

static bool validateDownCount(struct lora_mac *self, uint16_t counter);

/* functions **********************************************************/

void MAC_init(struct lora_mac *self, enum lora_region_id region, struct lora_radio *radio)
{
    LORA_PEDANTIC(self != NULL)
    LORA_PEDANTIC(radio != NULL)

    (void)memset(self, 0, sizeof(*self));
    
    self->radio = radio;    
    self->region = Region_getRegion(region);
    
    Event_init(&self->events);
    
    Radio_setEventHandler(self->radio, self, MAC_radioEvent);

    Region_getDefaultChannels(self->region, self, addDefaultChannel);    
}

bool MAC_send(struct lora_mac *self, bool confirmed, uint8_t port, const void *data, uint8_t len)
{
    LORA_PEDANTIC(self != NULL)
    LORA_PEDANTIC((len == 0) || (data != NULL))
    
    bool retval = false;
    
    struct lora_frame_data f;
    uint8_t key[16U];
    uint64_t timeNow = System_getTime();
    
    if(self->status.personalised || self->status.joined){
    
        /* stack must be idle (i.e. not sending) */
        if(self->state == IDLE){
        
            if((port > 0U) && (port <= 223U)){
                
                if(len <= Region_getPayload(self->region, System_getTXRate(self))){

                    if(selectChannel(self, timeNow, System_getTXRate(self), &self->tx.chIndex, &self->tx.freq)){
                
                        f.devAddr = System_getDevAddr(self);
                        f.counter = System_incrementUp(self);
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
                            
                            System_getNwkSKey(self, key);
                        }
                        else{
                            
                            System_getAppSKey(self, key);
                        }
                        
                        self->bufferLen = Frame_putData(FRAME_TYPE_DATA_UNCONFIRMED_UP, key, &f, self->buffer, sizeof(self->buffer));
                        
                        (void)Event_onTimeout(&self->events, System_getTime(), self, tx);
                        
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
        
        LORA_ERROR("stack must be personalised or joined before sending data")
    }
    
    return retval;
}

bool MAC_join(struct lora_mac *self)
{
    bool retval = false;
    
    uint64_t timeNow = System_getTime();
    
    if(!self->status.personalised){
    
        if(self->state == IDLE){

            if(selectChannel(self, timeNow, System_getTXRate(self), &self->tx.chIndex, &self->tx.freq)){

                struct lora_frame_join_request f;      
                uint8_t appKey[16U];      
                
                System_getAppKey(self, appKey);
                
                System_getAppEUI(self, f.appEUI);
                System_getDevEUI(self, f.devEUI);            
                
                f.devNonce = System_rand();
                f.devNonce <<= 8;
                f.devNonce |= System_rand();

                self->bufferLen = Frame_putJoinRequest(appKey, &f, self->buffer, sizeof(self->buffer));
                
                (void)Event_onTimeout(&self->events, System_getTime(), self, tx);
                
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
        
        LORA_ERROR("stack has been personalised - join is not possible")
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
    
        System_setDevAddr(self, devAddr);
        System_setNwkSKey(self, nwkSKey);
        System_setAppSKey(self, appSKey);
     
        retval = true;
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
        uint32_t Tpreamble = (Ts * 12U) +  (Ts / 4U);       //preamble (us)

        uint32_t numerator = (8U * (uint32_t)payloadLen) - (4U * (uint32_t)sf) + 28U + ( crc ? 16U : 0U ) - ( (header) ? 20U : 0U );
        uint32_t denom = 4U * ((uint32_t)sf - ( lowDataRateOptimize ? 2U : 0U ));

        uint32_t Npayload = 8U + ((numerator / denom) + (((numerator % denom) != 0) ? 1U : 0U)) * ((uint32_t)CR_5 + 4U);

        uint32_t Tpayload = Npayload * Ts;

        Tpacket = Tpreamble + Tpayload;
    }

    return Tpacket;
}

/* static functions ***************************************************/

static void tx(void *receiver, uint64_t time)
{
    LORA_PEDANTIC(receiver != NULL)
    
    struct lora_mac *self = (struct lora_mac *)receiver;        
    struct lora_data_rate rate_setting;
    
    if(Region_getRate(self->region, System_getTXRate(self), &rate_setting)){
    
        struct lora_radio_tx_setting radio_setting = {
            
            .freq = self->tx.freq,
            .bw = rate_setting.bw,
            .sf = rate_setting.sf,
            .cr = CR_5,
            .power = System_getTXPower(self)         
        };
    
        LORA_PEDANTIC((self->state == JOIN_WAIT_TX) || (self->state == WAIT_TX))
    
        if(Radio_transmit(self->radio, &radio_setting, self->buffer, self->bufferLen)){

            registerTime(self, self->tx.freq, System_getTime(), MAC_calculateOnAirTime(radio_setting.bw, radio_setting.sf, self->bufferLen));
            
            self->state = TX;
                
            self->txComplete = Event_onInput(&self->events, EVENT_TX_COMPLETE, self, txComplete);
        
            self->resetRadio = NULL;    //todo: add a watchdog to catch situation where IO lines don't work as expected    
        }
        else{
            
            LORA_ERROR("radio reject parameters")            
            abandonSequence(self);
        }
    }
    else{
        
        LORA_ERROR("invalid rate")            
        abandonSequence(self);
    }
}

static void txComplete(void *receiver, uint64_t time)
{
    LORA_PEDANTIC(receiver != NULL)
    
    struct lora_mac *self = (struct lora_mac *)receiver;            
    uint64_t timeNow = System_getTime();
    uint64_t futureTime;
        
    LORA_PEDANTIC((self->state == JOIN_TX) || (self->state == TX))
    
    Radio_sleep(self->radio);
    
    self->txCompleteTime = time;
        
    futureTime = self->txCompleteTime + ((self->state == TX) ? System_getRX1Delay(self) : Region_getJA1Delay(self->region));
    
    self->state = (self->state == TX) ? WAIT_RX1 : JOIN_WAIT_RX1;                

    if(futureTime > timeNow){

        (void)Event_onTimeout(&self->events, futureTime, self, rxStart);
    }
    else{
        
        LORA_ERROR("missed RX slot")        
        rxFinish(self);
    }         
}

static void rxStart(void *receiver, uint64_t time)
{
    LORA_PEDANTIC(receiver != NULL)
    
    struct lora_mac *self = (struct lora_mac *)receiver;    
    struct lora_radio_rx_setting radio_setting;
    
    uint8_t rate;
    uint32_t freq;
    struct lora_data_rate rate_setting;
    
    uint64_t targetTime;
    uint64_t timeNow = System_getTime();
        
    LORA_PEDANTIC((self->state == WAIT_RX1) || (self->state == WAIT_RX2) || (self->state == JOIN_WAIT_RX1) || (self->state == JOIN_WAIT_RX2))
    LORA_PEDANTIC(time <= timeNow)
   
    targetTime = timeNow - System_getRX2Delay(self);
    
    if(targetTime <= self->txCompleteTime){
        
        /* wait until the deadline - todo: add a faff margin */
        if(targetTime < self->txCompleteTime){
            
            System_usleep(self->txCompleteTime - targetTime);
        }
        
        switch(self->state){
        default:
        case JOIN_WAIT_RX1:
        case WAIT_RX1:
        
            (void)Region_getRX1DataRate(self->region, System_getTXRate(self), System_getRX1DROffset(self), &rate);
            (void)Region_getRX1Freq(self->region, self->tx.freq, &freq);
            
            self->state = (self->state == WAIT_RX1) ? RX1 : JOIN_RX1;                            
            break;
        
        case JOIN_WAIT_RX2:    
        case WAIT_RX2:    
        
            rate = System_getRX2DataRate(self);    
            freq = System_getRX2Freq(self);
        
            self->state = (self->state == WAIT_RX2) ? RX2 : JOIN_RX2;                
            break;
        }
        
        (void)Region_getRate(self->region, rate, &rate_setting);
        
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
        (void)Event_onTimeout(&self->events, self->txCompleteTime + System_getRX2Delay(self), self, rxStart);
        break;
    
    case JOIN_RX1:        
        self->state = JOIN_WAIT_RX2;    
        (void)Event_onTimeout(&self->events, self->txCompleteTime + Region_getJA2Delay(self->region), self, rxStart);
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
        
    System_getAppKey(self, appKey);
    System_getAppSKey(self, appSKey);
    System_getNwkSKey(self, nwkSKey);
    
    self->bufferLen = Radio_collect(self->radio, self->buffer, sizeof(self->buffer));        
    
    if(Frame_decode(appKey, nwkSKey, appSKey, self->buffer, self->bufferLen, &result)){
        
        switch(result.type){
        case FRAME_TYPE_JOIN_ACCEPT:
        
            if(self->status.joinPending){
                
                self->status.joinPending = false;
                self->status.joined = true;                

                System_resetUp(self);
                System_resetDown(self);
                
                System_setRX1DROffset(self, result.fields.joinAccept.rx1DataRateOffset);
                System_setRX2DataRate(self, result.fields.joinAccept.rx2DataRate);

                if(result.fields.joinAccept.cfListPresent){
                    
                    if(Region_isDynamic(self->region)){
   
                        size_t i;
                    
                        for(i=0U; i < sizeof(result.fields.joinAccept.cfList)/sizeof(*result.fields.joinAccept.cfList); i++){
                            
                            uint8_t chIndex = 4U + i;
                            
                            //Region_validateFreq(self, chIndex, result.fields.joinAccept.cflist[i]);
                            
                            (void)System_setChannel(self, chIndex, result.fields.joinAccept.cfList[i], 0U, 5U);
                        }                           
                    }                    
                }   
            }
            else{
                
                LORA_ERROR("ignoring a JOIN Accept we didn't ask for")
            }
            break;
        
        case FRAME_TYPE_DATA_UNCONFIRMED_DOWN:
        case FRAME_TYPE_DATA_CONFIRMED_DOWN:
            
            if(self->status.personalised){
            
                if(System_getDevAddr(self) == result.fields.data.devAddr){
                
                    if(validateDownCount(self, result.fields.data.counter)){
                
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

static void handleCommands(void *receiver, const struct lora_downstream_cmd *cmd)
{
    struct lora_mac *self = (struct lora_mac *)receiver;
    
    switch(cmd->type){
    default:
    case LINK_CHECK:
    
        //System_logLinkStatus(self, cmd->fields.linkCheckAns.margin, cmd->fields.linkCheckAns.gwCount);
        //maybe reply?
        break;
        
    case LINK_ADR:                    
        //these need be processed as a transaction
        break;
    
    case DUTY_CYCLE:                
        
        //System_setMaxDutyCycle(self, cmd->fields.dutyCycleReq.maxDutyCycle);
        break;
    
    case RX_PARAM_SETUP:
        
        break;
    
    case DEV_STATUS:
        
        break;
    
    case NEW_CHANNEL:    
    {
        if(Region_isDynamic(self->region)){
        
            //struct lora_new_channel_ans ans;
        
            //ans.rateOK = Region_validateRate(self->region, chIndex, minRate, maxRate);        
            //ans.freqOK = Region_validateFreq(self->region, chIndex, freq);
            
            //if(ans.rateOK && ans.freqOK){
                
              //  (void)System_addChannel(self, chIndex, freq, minRate, maxRate);                        
            //}
            
            //make reply?
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
    if(!System_setChannel(receiver, chIndex, freq, minRate, maxRate)){
        
        LORA_ERROR("could not add default channel")
        LORA_ASSERT(false)
    }
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
    
    if(!System_channelIsMasked(self, chIndex)){
    
        if(getChannel(self, chIndex, &freq, &minRate, &maxRate)){
            
            if(Region_getBand(self->region, freq, &band)){
            
                if(timeNow <= self->bands[band]){
                
                    if(rate >= minRate && rate <= maxRate){
                   
                        retval = true;
                    }   
                }
            }
        }
    }
    
    return retval;
}

static bool getChannel(struct lora_mac *self, uint8_t chIndex, uint32_t *freq, uint8_t *minRate, uint8_t *maxRate)
{
    bool retval = false;
    
    if(Region_isDynamic(self->region)){
        
        retval = System_getChannel(self, chIndex, freq, minRate, maxRate);                        
    }
    else{
        
        retval = Region_getChannel(self->region, chIndex, freq, minRate, maxRate);                        
    }
     
    return retval;
}

static bool validateDownCount(struct lora_mac *self, uint16_t counter)
{
    bool retval = false;
    
    if((uint32_t)counter < ((uint32_t)System_getDown(self) + (uint32_t)Region_getMaxFCNTGap(self->region))){
        
        retval = true;
        System_setDown(self, counter);
    }
    
    return retval;
}
