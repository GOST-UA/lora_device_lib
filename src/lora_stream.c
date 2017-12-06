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

#include "lora_stream.h"
#include "lora_debug.h"

#include <string.h>

struct lora_stream * Stream_init(struct lora_stream *self, void *buf, size_t size)
{
    LORA_PEDANTIC(self != NULL)
    LORA_PEDANTIC((buf != NULL) || (size == 0U))
    
    self->write = buf;
    self->read = buf;
    self->size = size;
    self->pos = 0U;
    
    return self;
}

struct lora_stream * Stream_initReadOnly(struct lora_stream *self, const void *buf, size_t size)
{
    LORA_PEDANTIC(self != NULL)
    LORA_PEDANTIC((buf != NULL) || (size == 0U))
    
    self->write = NULL;
    self->read = buf;
    self->size = size;
    self->pos = 0U;
    
    return self;
}

bool Stream_read(struct lora_stream *self, void *buf, size_t count)
{
    LORA_PEDANTIC(self != NULL)
    LORA_PEDANTIC(buf != NULL)
    
    bool retval = false;
    
    if((self->size - self->pos) >= count){
    
        (void)memcpy(buf, &self->read[self->pos], count);
        self->pos += count;
        retval = true;
    }    
    
    return retval;
}

bool Stream_write(struct lora_stream *self, const void *buf, size_t count)
{
    LORA_PEDANTIC(self != NULL)
    
    bool retval = false;
    
    if((self->write != NULL) && (self->size - self->pos) >= count){
        
        (void)memcpy(&self->write[self->pos], buf, count);
        self->pos += count;
        retval = true;
    }    
    
    return retval;
}

size_t Stream_tell(const struct lora_stream *self)
{
    return self->pos;
}

size_t Stream_remaining(const struct lora_stream *self)
{
    return self->size - self->pos;
}

