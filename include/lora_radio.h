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

#ifndef LORA_RADIO_H
#define LORA_RADIO_H

#include "lora_radio_defs.h"
#include <stdint.h>
#include <stdbool.h>

struct lora_mac;

struct lora_board {

    void *receiver;
    void (*select)(void *receiver, bool state);     /**< select the chip (this should also 'take' the resource if it's shared) */
    void (*reset)(void *receiver, bool state);      /**< true will hold the device in reset */
    void (*reset_wait)(void *receiver);             /**< block for an appropriate amount of time for reset to take effect */
    void (*write)(void *receiver, uint8_t data);    /**< write a byte */
    uint8_t (*read)(void *receiver);                /**< read a byte */
};

enum lora_radio_event {
    
    LORA_RADIO_TX_COMPLETE,
    LORA_RADIO_RX_READY,
    LORA_RADIO_RX_TIMEOUT
};

typedef void (*radioEventCB)(void *receiver, enum lora_radio_event event, uint64_t time);

struct lora_radio {
    enum lora_radio_type {
        LORA_RADIO_SX1272
    }
    type;
    struct lora_board board;
    void *eventReceiver;
    radioEventCB eventHandler;
    union {
        struct {

        } sx1272;
    }
    state;
};

struct lora_radio_if {
    void (*init)(struct lora_radio *self, enum lora_radio_type type, const struct lora_board *board);
    void (*transmit)(struct lora_radio *self, const void *data, uint8_t len);
    uint8_t (*collect)(struct lora_radio *self, void *data, uint8_t max);
    bool (*setParameters)(struct lora_radio *self, uint32_t freq, enum lora_signal_bandwidth bw, enum lora_spreading_factor sf);
    void (*raiseInterrupt)(struct lora_radio *self, uint8_t n, uint64_t time);
};

void LoraRadio_init(struct lora_radio *self, enum lora_radio_type type, const struct lora_board *board);
void LoraRadio_reset(struct lora_radio *self);
void LoraRadio_transmit(struct lora_radio *self, const void *data, uint8_t len);
void LoraRadio_receive(struct lora_radio *self);
uint8_t LoraRadio_collect(struct lora_radio *self, void *data, uint8_t max);
bool LoraRadio_setParameters(struct lora_radio *self, uint32_t freq, enum lora_signal_bandwidth bw, enum lora_spreading_factor sf);
void LoraRadio_interrupt(struct lora_radio *self, uint8_t n, uint64_t time);
void LoraRadio_setEventHandler(struct lora_radio *self, void *receiver, radioEventCB cb);

#endif
