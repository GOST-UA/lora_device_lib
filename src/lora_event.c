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
#include <string.h>

uint32_t getTime(void);


/* functions **********************************************************/

void Event_init(struct lora_event *self)
{
    (void)memset(self, 0, sizeof(*self));
}

void Event_receive(struct lora_event *self, enum on_input_types type, uint32_t time)
{
    if(!self->onInput[type].state){
    
        self->onInput[type].time = time;
        self->onInput[type].state = true;
    }        
}

void Event_tick(struct lora_event *self)
{
    uint32_t time = getTime();
    size_t i;
    
    for(i=0U; i < sizeof(self->onInput)/sizeof(*self->onInput); i++){
        
        if((self->onInput[i].handler != NULL) && self->onInput[i].state){
            
            LORA_ASSERT(time >= self->onInput[i].time)
            
            event_handler_t handler = self->onInput[i].handler;
            self->onInput[i].handler = NULL;
            
            handler(self->onInput[i].receiver, time - self->onInput[i].time);       
        }
    }
    
    struct on_timeout *ptr = self->head;
    
    while(ptr != NULL){
        
        if(time >= ptr->time){
            
            event_handler_t handler = ptr->handler;
            void *receiver = ptr->receiver;
            
            self->head = self->head->next;
            ptr->next = self->free;
            self->free = ptr;
            
            handler(receiver, time - ptr->time);
        }
        else{
            
            break;
        }        
    }
}

void *Event_onTimeout(struct lora_event *self, uint32_t timeout, void *receiver, event_handler_t handler)
{
    void *retval = NULL;
    
    uint32_t time = getTime();
    
    if(self->free != NULL){

        struct on_timeout *to = self->free;
        self->free = self->free->next;
        
        to->time = time;
        to->handler = handler;
        to->receiver = receiver;

        if(self->head == NULL){

            self->head = to;
        }
        else{

            struct on_timeout *ptr = self->head;

            do{

                if(time < ptr->time){

                    break;
                }
                else{

                    ptr = ptr->next;
                }
            }
            while(ptr->next != NULL);
        }

        retval = (void *)to;
    }

    return retval;
}

void *Event_onInput(struct lora_event *self, enum on_input_types event, void *receiver, event_handler_t handler)
{
    self->onInput[event].state = false; // this needs to be atomic
    self->onInput[event].handler = handler;
    self->onInput[event].receiver = receiver;
    
    return (void *)&self->onInput[event];
}

void Event_cancel(struct lora_event *self, void **event)
{
    size_t i;
    
    if((event != NULL) && (*event != NULL)){
        
        for(i=0U; i < sizeof(self->onInput)/sizeof(*self->onInput); i++){
                
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






