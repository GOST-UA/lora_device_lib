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
    
    LORA_MAC_READY,         /**< previous request succeeded; MAC is ready for next request */
    LORA_MAC_TIMEOUT,       /**< previous confirmed data or send request timed out */
    LORA_MAC_RX,            /**< data received */    
};

enum lora_mac_operation {
    LORA_MAC_UNCONFIRMED_DATA,
    LORA_MAC_CONFIRMED_DATA,
    LORA_MAC_JOIN,
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
    
    WAIT_TX,    // waiting for channel to become available
    TX,         // radio is TX
    WAIT_RX1,   // waiting for first RX window
    RX1,        // first RX window
    WAIT_RX2,   // waiting for second RX window
    RX2,        // second RX window
    
    RESET_WAIT, // waiting for reset
    
    ERROR
};

struct lora_mac {

    enum lora_mac_state state;
    enum lora_mac_operation op;
    
    struct {
        
        bool joined : 1U;           /**< MAC is in a joined state */
        bool joining : 1U;          /**< MAC is waiting to join */
        bool personalized : 1U;     /**< MAC has been activated by personalization */
        bool confirmPending : 1U;
        bool confirmed : 1U;
    
    } status;
    
    /** single buffer for sending and receiving */
    uint8_t buffer[UINT8_MAX];
    
    /** size of message in `buffer` */
    uint8_t bufferLen;
    
    /** tracks system time for when each band will become available */
    uint64_t bands[6U];
    
    uint16_t devNonce;
    
    struct {
        
        uint8_t chIndex;
        uint32_t freq;
        
    } tx;
    
    #define RX_WDT_INTERVAL 60000
    
    struct lora_radio *radio;           /**< radio belonging to MAC */
    struct lora_event events;           /**< event manager */
    const struct lora_region *region;   /**< region MAC is oeprating in */
    
    //void *rx1Ready;     /**< timer callback for RX1 window start */
    void *rx2Ready;     /**< timer callback for RX2 window start */
    
    void *rxComplete;   /**< RX complete IO event */
    void *rxTimeout;    /**< RX timeout IO event */
    
    lora_mac_response_fn responseHandler;
    void *responseReceiver;
    
    void *system;       /**< passed as receiver in every System_* call */
};

void MAC_init(struct lora_mac *self, void *system, enum lora_region_id region, struct lora_radio *radio, void *receiver, lora_mac_response_fn cb);

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

void MAC_radioEvent(void *receiver, enum lora_radio_event event, uint64_t time);

uint32_t MAC_calculateOnAirTime(enum lora_signal_bandwidth bw, enum lora_spreading_factor sf, uint8_t payloadLen);

/** Drive the MAC
 * 
 * @param[in] self
 * 
 * */
void MAC_tick(struct lora_mac *self);

/** Get number of ticks until the next event
 * 
 * @param[in] self
 * @return ticks until next event
 * 
 * @retval UINT64_MAX there are no future events at this time
 * 
 * */
uint64_t MAC_ticksUntilNextEvent(struct lora_mac *self);

/** Is MAC joined?
 * 
 * @param[in] self
 * @return true if MAC is joined to a network
 * 
 * */
bool MAC_isJoined(struct lora_mac *self);

/** Restore all network configurable parameters to region defaults
 * 
 * @param[in] self
 * 
 * */
void MAC_restoreDefaults(struct lora_mac *self);

/** Get number of ticks until next channel is ready
 * 
 * @param[in] self
 * 
 * */
uint64_t MAC_ticksUntilNextChannel(struct lora_mac *self);

#ifdef __cplusplus
}
#endif

#endif
