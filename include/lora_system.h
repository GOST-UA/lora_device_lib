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

#ifndef LORA_SYSTEM_H
#define LORA_SYSTEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>


/** Retrieve system time in microseconds
 * 
 * @return system time in microseconds
 * 
 * */
uint64_t System_getTime(void);

/** Halt the program counter for interval microseconds
 * 
 * @param[in] interval microseconds
 * 
 * */
void System_usleep(uint32_t interval);

/** Set the value of receiver to value in an atomic operation
 * 
 * @param[out] receiver
 * @param[in] value
 * 
 * */
void System_atomic_setPtr(void **receiver, void *value);

void System_rand(uint8_t *data, size_t len);

/** Retrieve the Application EUI
 * 
 * @param[in] owner the owner of this EUI
 * @param[out] eui 8 byte buffer
 * 
 * */
void System_getAppEUI(void *owner, uint8_t *eui);

/** Retrieve the Device EUI
 *
 * @param[in] owner the owner of this EUI
 * @param[out] eui 8 byte buffer
 * 
 * */
void System_getDevEUI(void *owner, uint8_t *eui);

/** Retrieve the Application Key
 *
 * @param[in] owner the owner of this AppKey
 * @param[out] key 16 byte buffer
 * 
 * */
void System_getAppKey(void *owner, uint8_t *key);

#ifdef __cplusplus
}
#endif

#endif
