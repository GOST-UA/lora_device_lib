/* Copyright (c) 2017-2018 Cameron Harper
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

static void tx(void *receiver, uint64_t time, uint64_t error);
static void txComplete(void *receiver, uint64_t time, uint64_t error);

static void rxStart(void *receiver, uint64_t time, uint64_t error);
static void rxReady(void *receiver, uint64_t time, uint64_t error);
static void rxTimeout(void *receiver, uint64_t time, uint64_t error);
static void rxFinish(struct lora_mac *self);

//static void resetRadio(void *receiver, uint64_t time);
static bool collect(struct lora_mac *self, struct lora_frame *frame);

static void abandonSequence(struct lora_mac *self);

static void handleCommands(void *receiver, const struct lora_downstream_cmd *cmd);
static void processCommands(struct lora_mac *self, const uint8_t *data, uint8_t len);

static bool selectChannel(struct lora_mac *self, uint64_t timeNow, uint8_t rate, uint8_t *chIndex, uint32_t *freq);
static void registerTime(struct lora_mac *self, uint32_t freq, uint64_t timeNow, uint32_t airTime);
static void addDefaultChannel(void *receiver, uint8_t chIndex, uint32_t freq, uint8_t minRate, uint8_t maxRate);
static bool getChannel(struct lora_mac *self, uint8_t chIndex, uint32_t *freq, uint8_t *minRate, uint8_t *maxRate);
static bool isAvailable(struct lora_mac *self, uint8_t chIndex, uint64_t timeNow, uint8_t rate);

static uint64_t timeBase(uint8_t value);

static uint64_t timeNextAvailable(struct lora_mac *self, uint64_t timeNow, uint8_t rate);

static uint32_t transmitTime(enum lora_signal_bandwidth bw, enum lora_spreading_factor sf, uint8_t size, bool crc);

//static void restoreDefaults(struct lora_mac *self);

/* functions **********************************************************/

void MAC_init(struct lora_mac *self, void *system, enum lora_region_id region, struct lora_radio *radio, void *receiver, lora_mac_response_fn cb)
{
    LORA_PEDANTIC(self != NULL)
    LORA_PEDANTIC(radio != NULL)
    LORA_PEDANTIC(cb != NULL)

    (void)memset(self, 0, sizeof(*self));
    
    self->system = system;
    self->radio = radio;    
    self->region = Region_getRegion(region);
    self->responseHandler = cb;
    self->responseReceiver = receiver;
    
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
    uint64_t timeNow = System_time();
    
    if(self->status.joined){
    
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
                        
                        self->op = confirmed ? LORA_OP_DATA_CONFIRMED : LORA_OP_DATA_UNCONFIRMED;
                        
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
        
        LORA_ERROR("stack must be joined before sending data")
    }
    
    return retval;
}

bool MAC_join(struct lora_mac *self)
{
    bool retval = false;
    
    uint64_t timeNow = System_time();
    
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
            
            self->state = WAIT_TX;
            self->op = LORA_OP_JOINING;
            
            retval = true;        
        }
        else{
            
            LORA_ERROR("no channel available")
        }
    }
    else{
        
        LORA_ERROR("ldl must be in IDLE state to perform join")
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

uint32_t MAC_transmitTimeUp(enum lora_signal_bandwidth bw, enum lora_spreading_factor sf, uint8_t size)
{
    return transmitTime(bw, sf, size, true);
}

uint32_t MAC_transmitTimeDown(enum lora_signal_bandwidth bw, enum lora_spreading_factor sf, uint8_t size)
{
    return transmitTime(bw, sf, size, false);
}

void MAC_tick(struct lora_mac *self)
{
    Event_tick(&self->events);
}

uint64_t MAC_ticksUntilNextEvent(struct lora_mac *self)
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

uint64_t MAC_ticksUntilNextChannel(struct lora_mac *self)
{
    uint64_t timeNow = System_time();
    uint64_t retval;
    
    retval = timeNextAvailable(self, timeNow, System_getTXRate(self->system));
    
    if(retval != UINT64_MAX){
        
        retval = retval - timeNow;
    }
    
    return retval;
}

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
    
    System_setTXPower(self->system, defaults.tx_power);
    System_setTXRate(self->system, defaults.tx_rate);
}

/* static functions ***************************************************/

#if 0
static uint32_t bwToNumber(enum lora_signal_bandwidth bw)
{
    uint32_t retval;
    
    switch(bw){
    case BW_125:
        retval = 125000U;
        break;        
    case BW_250:
        retval = 125000U;
        break;            
    case BW_500:
        retval = 125000U;
        break;            
    default:
    case BW_FSK:
        retval = 0U;
        break;
    }
    
    return retval;
}
#endif

static uint32_t transmitTime(enum lora_signal_bandwidth bw, enum lora_spreading_factor sf, uint8_t size, bool crc)
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
     * - calculated in microseconds then converted to ticks (usually either tens of us, or us)
     * 
     * */

    uint32_t Tpacket = 0U;

    if((bw != BW_FSK) && (sf != SF_FSK)){

        // for now hardcode this according to this recommendation
        bool lowDataRateOptimize = ((bw == BW_125) && ((sf == SF_11) || (sf == SF_12))) ? true : false;
        //bool crc = true;    // true for uplink, false for downlink
        bool header = true; 

        uint32_t Ts = ((1U << sf) * 1000000U) / (uint32_t)bw;     //symbol rate (us)
        uint32_t Tpreamble = (Ts * 12U) +  (Ts / 4U);   //preamble (us)

        uint32_t numerator = (8U * (uint32_t)size) - (4U * (uint32_t)sf) + 28U + ( crc ? 16U : 0U ) - ( header ? 20U : 0U );
        uint32_t denom = 4U * ((uint32_t)sf - ( lowDataRateOptimize ? 2U : 0U ));

        uint32_t Npayload = 8U + ((((numerator / denom) + (((numerator % denom) != 0U) ? 1U : 0U)) * ((uint32_t)CR_5 + 4U)));

        uint32_t Tpayload = Npayload * Ts;

        Tpacket = Tpreamble + Tpayload;
    }

    return Tpacket / (1000000U/LORA_TIMEBASE);
}

static void tx(void *receiver, uint64_t time, uint64_t error)
{
    struct lora_mac *self = (struct lora_mac *)receiver;            
    
    LORA_PEDANTIC(receiver != NULL)
    LORA_PEDANTIC(self->op != LORA_OP_NONE)
    
    struct lora_data_rate rate_setting;
    
    if(Region_getRate(self->region, System_getTXRate(self->system), &rate_setting)){
    
        struct lora_radio_tx_setting radio_setting = {
            
            .freq = self->tx.freq,
            .bw = rate_setting.bw,
            .sf = rate_setting.sf,
            .cr = CR_5,
            .power = System_getTXPower(self->system),
            .preamble = 8U,
            .channel = self->tx.chIndex
        };
    
        LORA_PEDANTIC(self->state == WAIT_TX)
    
        if(Radio_transmit(self->radio, &radio_setting, self->buffer, self->bufferLen)){

            registerTime(self, self->tx.freq, System_time(), transmitTime(radio_setting.bw, radio_setting.sf, self->bufferLen, true));
            
            self->state = TX;
                
            (void)Event_onInput(&self->events, EVENT_TX_COMPLETE, self, txComplete);        
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

static void txComplete(void *receiver, uint64_t time, uint64_t error)
{
    struct lora_mac *self = (struct lora_mac *)receiver;            
    
    LORA_PEDANTIC(receiver != NULL)
    LORA_PEDANTIC(self->state == TX)
    LORA_PEDANTIC(self->op != LORA_OP_NONE)
    
    uint32_t rx1Time;
        
    Radio_sleep(self->radio);
    
    self->state = WAIT_RX1;                
    rx1Time = time + timeBase((self->op == LORA_OP_JOINING) ? Region_getJA1Delay(self->region) : System_getRX1Delay(self->system)) - error;

    (void)Event_onTimeout(&self->events, rx1Time, self, rxStart);    
    self->rx2Ready = Event_onTimeout(&self->events, rx1Time + timeBase(1U), self, rxStart);
}

static void rxStart(void *receiver, uint64_t time, uint64_t error)
{
    struct lora_mac *self = (struct lora_mac *)receiver;    
    
    LORA_PEDANTIC(receiver != NULL)
    LORA_PEDANTIC((self->state == WAIT_RX1) || (self->state == WAIT_RX2) || (self->state == RX2)||(self->state == RX1))
    LORA_PEDANTIC(self->op != LORA_OP_NONE)
    
    struct lora_radio_rx_setting radio_setting;
    
    uint8_t rate;
    uint32_t freq;
    struct lora_data_rate rate_setting;
    
    /* ignore RX2 if it fires while RX1 is active */
    if(self->state != RX1){
        
        #ifndef RX_MARGIN
        #define RX_MARGIN 100
        #endif
       
        if(error < RX_MARGIN){
        
            // if we are too early we can block (todo: or something else)
            if((RX_MARGIN - error) > 0U){
                
                System_sleep(RX_MARGIN - error);
            }
            
            switch(self->state){
            default:
            case WAIT_RX1:        
            
                (void)Region_getRX1DataRate(self->region, System_getTXRate(self->system), System_getRX1DROffset(self->system), &rate);
                (void)Region_getRX1Freq(self->region, self->tx.freq, &freq);            
                self->state = RX1;                            
                break;
            
            case WAIT_RX2:    
            
                rate = System_getRX2DataRate(self->system);    
                freq = System_getRX2Freq(self->system);        
                self->state = RX2;                
                break;
            }
            
            (void)Region_getRate(self->region, rate, &rate_setting);
            
            radio_setting.freq = freq;
            radio_setting.bw = rate_setting.bw;
            radio_setting.sf = rate_setting.sf;
            radio_setting.cr = CR_5;
            radio_setting.preamble = 8U;
            radio_setting.timeout = (rate_setting.sf <= SF_9) ? 8U : 5U;
            
            if(Radio_receive(self->radio, &radio_setting)){
                 
                self->rxComplete = Event_onInput(&self->events, EVENT_RX_READY, self, rxReady);        
                self->rxTimeout = Event_onInput(&self->events, EVENT_RX_TIMEOUT, self, rxTimeout);                            
            }
            else{
                
                LORA_INFO("could not apply radio settings")
                abandonSequence(self);
            }         
        }
        else{
            
            LORA_INFO("missed RX deadline")            
            
            rxFinish(self);            
        }
    }
}

static void rxReady(void *receiver, uint64_t time, uint64_t error)
{
    struct lora_mac *self = (struct lora_mac *)receiver;    
    struct lora_frame frame;
    
    LORA_PEDANTIC(receiver != NULL)
    LORA_PEDANTIC(self->op != LORA_OP_NONE)
    LORA_PEDANTIC((self->state == RX1) || (self->state == RX2))
        
    Event_cancel(&self->events, &self->rxTimeout);    
    
    Radio_sleep(self->radio);
    
    if(collect(self, &frame)){
        
        Event_cancel(&self->events, &self->rx2Ready);
        
        switch(self->op){
        default:
        case LORA_OP_NONE:
        case LORA_OP_DATA_UNCONFIRMED:
            self->responseHandler(self->responseReceiver, LORA_MAC_READY, NULL);            
            break;
        case LORA_OP_JOINING:
            self->responseHandler(self->responseReceiver, (frame.type == FRAME_TYPE_JOIN_ACCEPT) ? LORA_MAC_READY : LORA_MAC_TIMEOUT, NULL);
            break;    
        case LORA_OP_DATA_CONFIRMED:
            self->responseHandler(self->responseReceiver, (frame.fields.data.ack) ? LORA_MAC_READY : LORA_MAC_TIMEOUT, NULL);
            break;
        }
        
        self->state = IDLE;           
        self->op = LORA_OP_NONE;
    }
    else{
        
        rxFinish(self);                    
    }    
}

static void rxTimeout(void *receiver, uint64_t time, uint64_t error)
{
    struct lora_mac *self = (struct lora_mac *)receiver;    

    LORA_PEDANTIC(receiver != NULL)
    LORA_PEDANTIC((self->state == RX1) || (self->state == RX2))
    LORA_PEDANTIC(self->op != LORA_OP_NONE)
    
    Event_cancel(&self->events, &self->rxComplete);    
    
    Radio_sleep(self->radio);
        
    rxFinish(self);        
}

static void rxFinish(struct lora_mac *self)
{
    if(self->state == RX2){
        
        switch(self->op){
        default:
        case LORA_OP_NONE:
            break;
        case LORA_OP_DATA_UNCONFIRMED:
            self->responseHandler(self->responseReceiver, LORA_MAC_READY, NULL);            
            break;
        case LORA_OP_JOINING:
        case LORA_OP_DATA_CONFIRMED:
            self->responseHandler(self->responseReceiver, LORA_MAC_TIMEOUT, NULL);
            break;
        }
        
        self->state = IDLE;
        self->op = LORA_OP_NONE;       
    }
    else{
        
        self->state = WAIT_RX2;
    }                
}


#if 0
static void resetRadio(void *receiver, uint32_t error)
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
        
static bool collect(struct lora_mac *self, struct lora_frame *frame)
{
    bool retval = false;    
    
    uint8_t appKey[16U];
    uint8_t appSKey[16U];
    uint8_t nwkSKey[16U];
        
    System_getAppKey(self->system, appKey);
    System_getAppSKey(self->system, appSKey);
    System_getNwkSKey(self->system, nwkSKey);
    
    self->bufferLen = Radio_collect(self->radio, self->buffer, sizeof(self->buffer));        
    
    if(Frame_decode(appKey, nwkSKey, appSKey, self->buffer, self->bufferLen, frame)){
        
        if(frame->valid){
            
            switch(frame->type){
            case FRAME_TYPE_JOIN_ACCEPT:
                
                retval = true;
                
                if(self->op == LORA_OP_JOINING){
                    
                    self->status.joined = true;                

                    System_resetUp(self->system);
                    System_resetDown(self->system);
                    
                    MAC_restoreDefaults(self);
                    
                    System_setRX1DROffset(self->system, frame->fields.joinAccept.rx1DataRateOffset);
                    System_setRX2DataRate(self->system, frame->fields.joinAccept.rx2DataRate);

                    if(frame->fields.joinAccept.cfListPresent){
                        
                        if(Region_isDynamic(self->region)){
       
                            size_t i;
                        
                            for(i=0U; i < sizeof(frame->fields.joinAccept.cfList)/sizeof(*frame->fields.joinAccept.cfList); i++){
                                
                                uint8_t chIndex = 4U + i;
                                
                                //Region_validateFreq(self, chIndex, frame->fields.joinAccept.cflist[i]);
                                
                                (void)System_setChannel(self->system, chIndex, frame->fields.joinAccept.cfList[i], 0U, 5U);
                            }                           
                        }                    
                    }   
                    
                    struct lora_aes_ctx ctx;
                    LoraAES_init(&ctx, appKey);
                    
                    (void)memset(nwkSKey, 0U, sizeof(nwkSKey));
                    
                    nwkSKey[0] = 1U;                
                    nwkSKey[1] = frame->fields.joinAccept.appNonce;
                    nwkSKey[2] = frame->fields.joinAccept.appNonce >> 8;
                    nwkSKey[3] = frame->fields.joinAccept.appNonce >> 16;
                    nwkSKey[4] = frame->fields.joinAccept.netID;
                    nwkSKey[5] = frame->fields.joinAccept.netID >> 8;
                    nwkSKey[6] = frame->fields.joinAccept.netID >> 16;
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
                    
                    LORA_INFO("ignoring unexpected JOIN Accept")
                }
                break;
            
            case FRAME_TYPE_DATA_UNCONFIRMED_DOWN:
            case FRAME_TYPE_DATA_CONFIRMED_DOWN:
                
                if(self->status.joined){
                
                    if(System_getDevAddr(self->system) == frame->fields.data.devAddr){
                    
                        if(System_receiveDown(self->system, frame->fields.data.counter, Region_getMaxFCNTGap(self->region))){
                        
                            processCommands(self, frame->fields.data.opts, frame->fields.data.optsLen);
                            
                            if(frame->fields.data.data != NULL){
                    
                                if(frame->fields.data.port > 0U){
                                        
                                    union lora_mac_response_arg arg;
                                    
                                    arg.rx.data = frame->fields.data.data;
                                    arg.rx.len = frame->fields.data.dataLen;
                                    arg.rx.port = frame->fields.data.port;
                                        
                                    self->responseHandler(self->responseReceiver, LORA_MAC_RX, &arg);                                        
                                }
                                else{
                                                                    
                                    processCommands(self, frame->fields.data.data, frame->fields.data.dataLen);
                                }
                            }
                            
                            retval = true;
                        }
                        else{
                            
                            LORA_INFO("discarding frame for counter sync")
                        }
                    }
                    else{
                        
                        LORA_INFO("ignoring data frame with different devAddr")
                    }
                }
                else{
                    
                    LORA_INFO("ignoring data frame while in unjoined state")
                }
                break;
            
            case FRAME_TYPE_JOIN_REQ:
            case FRAME_TYPE_DATA_UNCONFIRMED_UP:
            case FRAME_TYPE_DATA_CONFIRMED_UP:            
            default:                
                LORA_INFO("received an upstream packet")            
                break;
            }
        }
    }
    
    return retval;
}

static void handleCommands(void *receiver, const struct lora_downstream_cmd *cmd)
{
    struct lora_mac *self = (struct lora_mac *)receiver;
    
    switch(cmd->type){
    default:
    case LINK_CHECK:
    
        System_setLinkStatus(self->system, cmd->fields.linkCheckAns.margin, cmd->fields.linkCheckAns.gwCount);
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

static uint64_t timeNextAvailable(struct lora_mac *self, uint64_t timeNow, uint8_t rate)
{
    uint8_t i;
    uint32_t freq;
    uint8_t minRate;
    uint8_t band = 0U;
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
    return ((uint64_t)value) * LORA_TIMEBASE;
}
