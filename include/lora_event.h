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

#ifndef LORA_EVENT_H
#define LORA_EVENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/** An event callback
 * 
 * @param[in] receiver
 * @param[in] time system time (ticks) event serviced
 * @param[in] error ticks passed since scheduled event time (i.e. event time == time - error)
 * 
 * */
typedef void (*event_handler_t)(void *receiver, uint64_t time, uint64_t error);

/** radio IO event source */
enum on_input_types {
  
    EVENT_TX_COMPLETE,
    EVENT_RX_READY,
    EVENT_RX_TIMEOUT,
    EVENT_NUM_EVENTS
};

/** IO event state */
struct on_input {
    
    event_handler_t handler;
    void *receiver;             
    bool state;                 // todo: make atomic or protect
    uint64_t time;              /**< time event received (us) */        
};

/** timer event state */
struct on_timeout {
    
    struct on_timeout *next;
        
    event_handler_t handler;
    void *receiver;
    uint64_t time;    
};

struct lora_event {

    #define EVENT_NUM_TIMERS 3U

    struct on_timeout pool[EVENT_NUM_TIMERS];
    struct on_timeout *free;
    struct on_timeout *head;

    struct on_input onInput[EVENT_NUM_EVENTS];
};

/** Init from mainloop
 * 
 * @param[in] self
 * 
 * */
void Event_init(struct lora_event *self);

/** Receive an event notification from ISR
 * 
 * @param[in] self
 * @param[in] type event source 
 * @param[in] time system time (us)
 * 
 * */
void Event_receive(struct lora_event *self, enum on_input_types type, uint64_t time);

/** Execute synchronous event loop from mainloop
 * 
 * - Call from "main loop" level task
 * - Will call callbacks previousl scheduled
 * - May schedule new events (from callbacks)
 * 
 * @param[in] self
 * 
 * */
void Event_tick(struct lora_event *self);

/** Schedule an IO event from mainloop
 * 
 * @param[in] self
 * @param[in] event event source
 * @param[in] receiver callback receiver
 * @param[in] handler callback handler
 * 
 * @return pointer to event state
 * 
 * @retval NULL event could not be scheduled
 * 
 * */
void *Event_onInput(struct lora_event *self, enum on_input_types event, void *receiver, event_handler_t handler);

/** Schedule a timer event from mainloop
 * 
 * @param[in] self
 * @param[in] timeout absolute system time (ticks) event will occur
 * @param[in] receiver callback receiver
 * @param[in] handler callback handler
 * 
 * @return pointer to event state
 * 
 * @retval NULL event could not be scheduled
 * 
 * */
void *Event_onTimeout(struct lora_event *self, uint64_t timeout, void *receiver, event_handler_t handler);

/** Cancel an event handler (and clear the reference) from mainloop
 * 
 * @param[in] self
 * @param[in] event pointer to pointer of event state
 * 
 * */
void Event_cancel(struct lora_event *self, void **event); 

/** Get interval until next event
 * 
 * @param[in] self
 * 
 * @return ticks
 * @retval UINT64_MAX unknown/io event
 * 
 * */
uint64_t Event_intervalUntilNext(struct lora_event *self);

#ifdef __cplusplus
}
#endif

#endif
