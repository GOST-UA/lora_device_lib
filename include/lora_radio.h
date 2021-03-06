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

#ifndef LORA_RADIO_H
#define LORA_RADIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lora_radio_defs.h"
#include <stdint.h>
#include <stdbool.h>

struct lora_board;

enum lora_radio_event {
    
    LORA_RADIO_TX_COMPLETE,
    LORA_RADIO_RX_READY,
    LORA_RADIO_RX_TIMEOUT
};

typedef void (*radioEventCB)(void *receiver, enum lora_radio_event event, uint64_t time);

struct lora_radio;

struct lora_radio_tx_setting {
    
    uint32_t freq;
    enum lora_signal_bandwidth bw;
    enum lora_spreading_factor sf;
    enum lora_coding_rate cr;
    uint16_t preamble;
    int power;    
    
    // fixme
    uint8_t channel;
};

struct lora_radio_rx_setting {
    
    uint32_t freq;
    enum lora_signal_bandwidth bw;
    enum lora_spreading_factor sf;
    enum lora_coding_rate cr;
    uint16_t preamble;
    uint16_t timeout;
    
    // fixme
    uint8_t channel;
};

/** Initialise self to default state
 * 
 * @warning this should be called once during stack initialisation
 * @warning this should not be called during operation to perform a device restart
 *
 * @note board should be buffered by Radio (so you don't need to keep it after making this call)
 * 
 * @param[in] self
 * @param[in] board board specific connections
 * 
 * */
struct lora_radio * Radio_init(struct lora_radio *self, const struct lora_board *board);

/** Set the state of the reset line for the purpose of performing POR
 * 
 * @param[in] self
 * @param[in] state true for reset active
 * 
 * */
void Radio_reset(struct lora_radio *self, bool state);

/** Put transeiver into sleep mode
 * 
 * @param[in] self
 * 
 * */
void Radio_sleep(struct lora_radio *self);

/** Setup radio to transmit
 * 
 * @param[in] self
 * @param[in] settings radio settings
 * @param[in] data buffer to send
 * @param[in] len byte length of data
 * 
 * @return true if transmit was started
 * 
 * */
bool Radio_transmit(struct lora_radio *self, const struct lora_radio_tx_setting *settings, const void *data, uint8_t len);

/** Setup radio to receive
 * 
 * @param[in] self
 * @param[in] settings radio settings 
 * 
 * @return true if receive window was opened
 * 
 * */
bool Radio_receive(struct lora_radio *self, const struct lora_radio_rx_setting *settings);

/** Collect message from radio
 * 
 * @param[in] self
 * @param[in] data buffer to write to
 * @param[in] max maximum byte length of data
 * 
 * @return bytes collected
 * 
 * */
uint8_t Radio_collect(struct lora_radio *self, void *data, uint8_t max);

/** Raise I/O interrupt from radio
 * 
 * @note called by ISR
 * 
 * @param[in] self
 * @param[in] n interrupt number
 * @param[in] time time of interrupt in us
 * 
 * */
void Radio_interrupt(struct lora_radio *self, uint8_t n, uint64_t time);

/** Set event handler 
 * 
 * @note called by MAC layer to set callback
 * 
 * @param[in] self
 * @param[in] receiver event receiver
 * @param[in[ cb handler method
 * 
 * */
void Radio_setEventHandler(struct lora_radio *self, void *receiver, radioEventCB cb);

/** Sample a 'random' byte from the radio (probably using some kind of wideband measurement)
 * 
 * @param[in] self
 * 
 * @return random byte
 * 
 * */
uint8_t Radio_getRandom(struct lora_radio *self);

#ifdef __cplusplus
}
#endif

#endif
