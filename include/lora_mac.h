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

#ifndef LORA_MAC_H
#define LORA_MAC_H

#include "lora_region.h"
#include "lora_radio.h"

#include <stdint.h>
#include <stdbool.h>

struct lora_event;
struct lora_channeL_list;
struct lora_mac;

/* shift to region */
#define INTERVAL_RX1    1000
#define INTERVAL_RX2    1000

enum mac_cmd_type {
    LINK_CHECK,
    LINK_ADR,
    DUTY_CYCLE,
    RX_PARAM_SETUP,
    DEV_STATUS,
    NEW_CHANNEL,
    RX_TIMING_SETUP,
    TX_PARAM_SETUP,
    DL_CHANNEL
};

enum lora_mac_response_type {
    LORA_MAC_TX_COMPLETE,
    LORA_MAC_TX_CONFIRMED,
    LORA_MAC_TX_TIMEOUT,
    LORA_MAC_RX,
    LORA_MAC_JOIN_SUCCESS,
    LORA_MAC_JOIN_TIMEOUT    
};

union lora_mac_response_arg {
    
    struct {
        
        uint8_t port;
        const uint8_t *data;
        uint8_t len;
        
    } rx;    
};

typedef void (*lora_mac_response_fn)(void *receiver, enum lora_mac_response_type type, const union lora_mac_response_arg *arg);

enum states {

    IDLE,
    
    RSSI_RANDOM,    // getting a 'random' seed from the radio
    
    WAIT_TX,
    TX,         // radio is TX
    WAIT_RX1,   // waiting for first RX window
    RX1,        // first RX window
    WAIT_RX2,   // waiting for second RX window
    RX2,        // second RX window
    
    JOIN_WAIT_TX,
    JOIN_TX,
    JOIN_WAIT_RX1,
    JOIN_RX1,
    JOIN_WAIT_RX2,
    JOIN_RX2,
    
    RESET_WAIT, // waiting for reset
    
    ERROR
};

#define MAX_FCNT_GAP 16384U

struct lora_mac {

    enum states state;
    
    uint8_t rate;
    uint8_t power;
    
    uint8_t previousRate;   /**< the previous rate used */
    uint32_t previousFreq;
    
    uint8_t txCount;

    uint8_t devEUI[8U];
    uint8_t appEUI[8U];
    
    uint8_t appKey[16U];

    uint8_t nwkSKey[16U];
    uint8_t appSKey[16U];
    uint32_t devAddr;

    uint16_t rwindow;
    uint16_t upCounter;
    uint16_t downCounter;
    
    uint16_t maxFrameCounterGap;
    
    uint16_t max_fcnt_gap;  /**< maximum frame counter gap */
    
    uint8_t rx1_delay;      /**< rx1 delay (seconds) */
    uint8_t rx2_delay;      /**< rx2 delay (seconds) */
    uint8_t rx2_rate;
    uint32_t rx2_freq;
    uint8_t rx1_offset;
    
    uint8_t ja1_delay;      /**< join accept 1 delay (seconds) */
    uint8_t ja2_delay;      /**< join accept 2 delay (seconds) */
    
    uint8_t adr_ack_limit;  
    uint8_t adr_ack_delay;  
    uint8_t adr_ack_timeout;    
    uint8_t adr_ack_dither;     
    
    uint64_t txCompleteTime;
    
    uint8_t buffer[UINT8_MAX];
    uint8_t bufferLen;
    
    uint8_t nbTrans;    /**< number of transmissions for each uplink message */
    
    bool joined;
    bool personalised;
    bool joinPending;
    bool confirmPending;
    bool confirmed;
    
    uint16_t devNonce;
    
    #define RX_WDT_INTERVAL 60000
    
    struct lora_channel_list *channels;
    struct lora_radio *radio;
    struct lora_event *events;
    
    /* these are references to events that we may want to cancel */
    void *rxReady;
    void *rxTimeout;
    void *txComplete;
    void *resetRadio;

    lora_mac_response_fn responseHandler;
    void *responseReceiver;
};

void MAC_init(struct lora_mac *self, struct lora_channel_list *channels, struct lora_radio *radio, struct lora_event *events);

/** Set join parameters locally
 * 
 * @param[in] self
 * @param[in] devAddr device address
 * @param[in] nwkSKey network session key
 * @param[in] appSKey application session key
 * 
 * @return true if personalization could be performed
 * 
 * */
bool MAC_personalize(struct lora_mac *self, uint32_t devAddr, const void *nwkSKey, const void *appSKey);

/** Set the number of times an upstream data frame will be sent (for redundancy)
 * 
 * (i.e. user asks to send a data frame, the stack may send it nbTrans times for redundancy)
 * 
 * 
 * 
 * */
bool MAC_setNbTrans(struct lora_mac *self, uint8_t nbTrans);

/** Send a message upstream
 * 
 * Call may be rejected for the following reasons:
 * 
 * - MAC is already sending
 * - Message is too large
 * 
 * If the call is accepted it may be some time before it completes. The
 * application will be notified of completion (and final status) via
 * the txCompleteHandler callback.
 * 
 * - Duty cycle limits on available channels
 * - Retransmission depending on MAC setting
 * 
 * @param[in] self
 * @param[in] confirmed true if this send should be confirmed
 * @param[in] port 
 * @param[in] data pointer to message to send (will be cached by MAC)
 * @param[in] len byte length of data
 * 
 * @retval true unconfirmed up is possible and now pending
 * 
 * */
bool MAC_send(struct lora_mac *self, bool confirmed, uint8_t port, const void *data, uint8_t len);


void MAC_setResponseHandler(struct lora_mac *self, void *receiver, lora_mac_response_fn cb);

bool MAC_join(struct lora_mac *self);

void MAC_radioEvent(void *receiver, enum lora_radio_event event, uint64_t time);

#endif
