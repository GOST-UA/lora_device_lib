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

#ifdef __cplusplus
extern "C" {
#endif

#include "lora_region.h"
#include "lora_radio.h"
#include "lora_event.h"

#include <stdint.h>
#include <stdbool.h>

struct lora_mac;

enum lora_mac_response_type {
    
    LORA_MAC_DATA_COMPLETE, /**< cycle is complete */ 
    LORA_MAC_DATA_TIMEOUT,  /**< cycle is complete but ack was not received */ 
    LORA_MAC_RX,            /**< received a data frame */
    LORA_MAC_JOIN_SUCCESS,  /**< sent join request, received join response in either RX1 or RX2 */
    LORA_MAC_JOIN_TIMEOUT   /**< sent join request, did not receive join response in either RX1 or RX2 */    
};

union lora_mac_response_arg {
    
    struct {
        
        uint8_t port;
        const uint8_t *data;
        uint8_t len;
        
    } rx;    
};

typedef void (*lora_mac_response_fn)(void *receiver, enum lora_mac_response_type type, const union lora_mac_response_arg *arg);

enum lora_mac_state {

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
    
    RESET_WAIT, // waiting for reset
    
    ERROR
};

struct lora_mac {

    enum lora_mac_state state;
    
    uint64_t txCompleteTime;
    
    uint8_t buffer[UINT8_MAX];
    uint8_t bufferLen;
    
    uint64_t bands[6U];
    
    uint16_t devNonce;
    
    struct {
        
        uint8_t chIndex;
        uint32_t freq;
        
    } tx;
    
    struct {
        
        bool joined : 1U;
        bool personalised : 1U;
        bool joinPending : 1U;
        bool confirmPending : 1U;
        bool confirmed : 1U;
    
    } status;
    
    #define RX_WDT_INTERVAL 60000
    
    struct lora_radio *radio;
    struct lora_event events;
    const struct lora_region *region;
    
    /* these are references to events that we may want to cancel */
    void *rxReady;
    void *rxTimeout;
    void *txComplete;
    void *resetRadio;

    lora_mac_response_fn responseHandler;
    void *responseReceiver;
};

void MAC_init(struct lora_mac *self, enum lora_region_id region, struct lora_radio *radio);

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

bool MAC_join(struct lora_mac *self);

bool MAC_setRate(struct lora_mac *self, uint8_t rate);
bool MAC_setPower(struct lora_mac *self, uint8_t power);

void MAC_setResponseHandler(struct lora_mac *self, void *receiver, lora_mac_response_fn cb);

void MAC_radioEvent(void *receiver, enum lora_radio_event event, uint64_t time);

uint32_t MAC_calculateOnAirTime(enum lora_signal_bandwidth bw, enum lora_spreading_factor sf, uint8_t payloadLen);

void MAC_tick(struct lora_mac *self);

uint64_t MAC_timeUntilNextEvent(struct lora_mac *self);

bool MAC_isJoined(struct lora_mac *self);
bool MAC_isPersonalised(struct lora_mac *self);


#ifdef __cplusplus
}
#endif

#endif
