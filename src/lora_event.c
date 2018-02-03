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

#include "lora_event.h"
#include "lora_debug.h"
#include "lora_system.h"
#include <string.h>


/* functions **********************************************************/

void Event_init(struct lora_event *self)
{
    size_t i;
    
    (void)memset(self, 0, sizeof(*self));
    
    self->free = self->pool;

    for(i=0U; i < sizeof(self->pool)/sizeof(*self->pool)-1U; i++){
        
        self->pool[i].next = &self->pool[i+1];    
    }
}

void Event_receive(struct lora_event *self, enum on_input_types type, uint64_t time)
{
    if(!self->onInput[type].state){
    
        self->onInput[type].time = time;
        self->onInput[type].state = true;
    }        
}

void Event_tick(struct lora_event *self)
{
    uint64_t time;
    size_t i;
    struct on_timeout to;
    struct on_timeout *ptr = self->head;
    struct on_timeout *prev = NULL;
    
    time = System_getTime();
    
    /* timeouts */
    while((ptr != NULL) && (time >= ptr->time)){
        
        to = *ptr;
            
        if(prev == NULL){
            
            self->head = ptr->next;
        }
        else{
            
            prev->next = ptr->next;                
        }
         
        ptr->next = self->free;
        self->free = ptr;         
        ptr = to.next;
        
        to.handler(to.receiver, to.time);        
    }
    
    /* io events */
    for(i=0U; i < sizeof(self->onInput)/sizeof(struct on_input); i++){
        
        if((self->onInput[i].handler != NULL) && self->onInput[i].state){
            
            LORA_ASSERT(time >= self->onInput[i].time)
            
            event_handler_t handler = self->onInput[i].handler;
            self->onInput[i].handler = NULL;
            
            handler(self->onInput[i].receiver, self->onInput[i].time);       
        }
    }
}

void *Event_onTimeout(struct lora_event *self, uint64_t timeout, void *receiver, event_handler_t handler)
{
    void *retval = NULL;
        
    if(self->free != NULL){

        struct on_timeout *to = self->free;
        struct on_timeout *ptr = self->head;
        struct on_timeout *prev = NULL;
        self->free = self->free->next;
        
        to->time = timeout;
        to->handler = handler;
        to->receiver = receiver;
        to->next = NULL;
        
        if(ptr == NULL){

            self->head = to;
        }
        else{

            while(ptr != NULL){

                if(to->time < ptr->time){
                    
                    if(prev == NULL){
                        
                        self->head = to;
                    }
                    else{
                        
                        prev->next = to;                                                        
                    }
                    
                    to->next = ptr;

                    break;
                }
                else{

                    prev = ptr;
                    ptr = ptr->next;
                }
            }
        }

        retval = (void *)to;
    }
    else{
        
        LORA_ERROR("timer pool exhausted")
    }
    

    return retval;
}

void *Event_onInput(struct lora_event *self, enum on_input_types event, void *receiver, event_handler_t handler)
{
    self->onInput[event].state = false;
    self->onInput[event].handler = handler;
    self->onInput[event].receiver = receiver;
    
    return (void *)&self->onInput[event];
}

void Event_cancel(struct lora_event *self, void **event)
{
    size_t i;
    
    if((event != NULL) && (*event != NULL)){
        
        for(i=0U; i < sizeof(self->onInput)/sizeof(struct on_input); i++){
                
            if(*event == &self->onInput[i]){
                
                self->onInput[i].handler = NULL;
                *event = NULL;
                break;    
            }            
        }
        
        if(*event != NULL){
            
            struct on_timeout *prev = NULL;
            struct on_timeout *ptr = self->head;
            
            while(ptr != NULL){
                
                if(*event == ptr){
                    
                    if(prev == NULL){
                        
                        self->head = ptr->next;                        
                    }
                    else{
                        
                        prev->next = ptr->next;
                    }
                    
                    ptr->next = self->free;                    
                    self->free = ptr;
                    break;
                }                
                else{
                    
                    prev = ptr;
                    ptr = ptr->next;
                }
            }
            
            *event = NULL;
        }        
    }
}

uint64_t Event_timeUntilNextEvent(struct lora_event *self)
{
    uint64_t retval;
    uint64_t time;
        
    retval = UINT64_MAX;
    
    if(self->head != NULL){
        
        time = System_getTime();
        
        if(self->head->time > time){
         
            retval = self->head->time - time;
        }
        else{
            
            retval = 0U;
        }        
    }
    
    return retval;
}
