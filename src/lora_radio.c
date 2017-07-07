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

#include "lora_radio.h"
#include "lora_debug.h"

extern const struct lora_radio_if LoraRadio_if_sx1272;

static const struct lora_radio_if *LoraRadio_if[] = {
    &LoraRadio_if_sx1272
};

/* functions **********************************************************/

void LoraRadio_init(struct lora_radio *self, enum lora_radio_type type, const struct lora_board *board)
{
    LORA_ASSERT(self != NULL)
    LORA_ASSERT(board != NULL)
    LORA_ASSERT(type < sizeof(LoraRadio_if)/sizeof(*LoraRadio_if))
    
    LoraRadio_if[type]->init(self, type, board);    
}

void LoraRadio_reset(struct lora_radio *self)
{
    self->board.reset(self->board.receiver, true);
    self->board.reset_wait(self->board.receiver);
    self->board.reset(self->board.receiver, false);
}

void LoraRadio_transmit(struct lora_radio *self, const void *data, uint8_t len)
{   
    LoraRadio_if[self->type]->transmit(self, data, len);
}

uint8_t LoraRadio_collect(struct lora_radio *self, void *data, uint8_t max)
{
    return LoraRadio_if[self->type]->collect(self, data, max);
}

bool LoraRadio_setParameters(struct lora_radio *self, uint32_t freq, enum signal_bandwidth bw, enum spreading_factor sf)
{
    return LoraRadio_if[self->type]->setParameters(self, freq, bw, sf);
}

void LoraRadio_interrupt(struct lora_radio *self, uint8_t n, uint64_t time)
{
    LoraRadio_if[self->type]->raiseInterrupt(self, n, time);
}

void LoraRadio_setEventHandler(struct lora_radio *self, void *receiver, radioEventCB cb)
{
    // todo: this is a critical section
    self->eventHandler = cb;
    self->eventReceiver = receiver;
}
