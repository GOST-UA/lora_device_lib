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

#include <stdint.h>
#include <stdbool.h>

struct lora_radio;
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

enum lora_tx_status {
    LORA_TX_COMPLETE,           ///< non-confirmed upstream tx operation is complete
    LORA_TX_CONFIRMED,          ///< tx operation complete and confirmation received
    LORA_TX_CONFIRM_TIMEOUT     ///< tx operation complete but confirmation never received
};

/** Transmit Complete Notification
 * 
 * @param[in] receiver
 * @param[in] status
 * 
 * */
typedef void (*txCompleteCB)(void *receiver, enum lora_tx_status status);

/** Receive Complete Notification
 * 
 * - receive must make a copy data or else it will be lost
 * 
 * @param[in] receiver
 * @param[in] port
 * @param[in] data
 * @param[in] len byte length of data
 * 
 * */
typedef void (*rxCompleteCB)(void *receiver, uint8_t port, const void *data, uint8_t len);

typedef void (*joinCB)(void *receiver, bool noResponse);

enum states {

    IDLE,
    
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
    
    ERROR
};

#define MAX_FCNT_GAP 16384U

struct lora_mac {

    enum states state;
    
    uint8_t txCount;

    uint8_t devEUI[8U];
    uint8_t appEUI[8U];
    
    uint8_t appKey[16U];

    uint8_t nwkSKey[16U];
    uint8_t appSKey[16U];

    uint16_t rwindow;
    uint16_t upCounter;
    uint16_t downCounter;
    uint16_t maxFrameCounterGap;
    uint8_t joinAcceptDelay1;   // fixed depending on region
    uint8_t joinAcceptDelay2;
    uint8_t receiveDelay1;      // adjustable by mac command
    uint8_t receiveDelay2;
    
    uint64_t txCompleteTime;
    
    uint8_t buffer[UINT8_MAX];
    uint8_t bufferLen;

    uint32_t devAddr;
    
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
    
    rxCompleteCB rxCompleteHandler;
    void *rxCompleteReceiver;
    
    txCompleteCB txCompleteHandler;
    void *txCompleteReceiver;
    
    joinCB joinHandler;
    void *joinReceiver;
};

void LoraMAC_init(struct lora_mac *self, struct lora_channel_list *channels, struct lora_radio *radio, struct lora_event *events);

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
bool LoraMAC_personalize(struct lora_mac *self, uint32_t devAddr, const void *nwkSKey, const void *appSKey);

/** Set the number of times an upstream data frame will be sent (for redundancy)
 * 
 * (i.e. user asks to send a data frame, the stack may send it nbTrans times for redundancy)
 * 
 * 
 * 
 * */
bool LoraMAC_setNbTrans(struct lora_mac *self, uint8_t nbTrans);

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
bool LoraMAC_send(struct lora_mac *self, bool confirmed, uint8_t port, const void *data, uint8_t len);

/** 
 * MAC will call this handler when a message is received downstream
 * 
 * @param[in] self
 * @param[in] user callback receiver
 * @param[in] cb handler
 * 
 * */
void LoraMAC_setReceiveHandler(struct lora_mac *self, void *receiver, rxCompleteCB cb);

/** 
 * MAC will call this handler when a send operation completes
 * 
 * @param[in] self
 * @param[in] user callback receiver
 * @param[in] cb handler
 * 
 * */
void LoraMAC_setTransmitHandler(struct lora_mac *self, void *receiver, txCompleteCB cb);


void LoraMAC_setJoinHandler(struct lora_mac *self, void *receiver, joinCB cb);

bool LoraMAC_join(struct lora_mac *self);

#endif
