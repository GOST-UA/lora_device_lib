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

#ifndef LORA_DEBUG_H
#define LORA_DEBUG_H

/* Include your own lora_debug.h file.
 * 
 * Within this file you could define things like LORA_ERROR and LORA_ASSERT.
 * 
 * */
#ifdef LORA_DEBUG_INCLUDE
#include LORA_DEBUG_INCLUDE
#endif

/* A printf-like function that captures run-time error level messages */
#ifndef LORA_ERROR
    #define LORA_ERROR(...)
#endif

/* A printf-like function that captures run-time info level messages */
#ifndef LORA_INFO
    #define LORA_INFO(...)
#endif

/* A printf-like function that captures run-time debug level messages */
#ifndef LORA_DEBUG
    #define LORA_DEBUG(...)
#endif

/* An assert-like function that performs run-time asserts on 'X' */
#ifndef LORA_ASSERT
    #define LORA_ASSERT(X)
#endif

/* A assert-like function that performs pedantic run-time asserts on 'X' */
#ifndef LORA_PEDANTIC
    #define LORA_PEDANTIC(X)
#endif

#endif
