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
#include <stdbool.h>
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

/** Retrieve the Application EUI
 * 
 * @param[in] receiver the receiver of this EUI
 * @param[out] eui 8 byte buffer
 * 
 * */
void System_getAppEUI(void *receiver, void *eui);

/** Retrieve the Device EUI
 *
 * @param[in] receiver the receiver of this EUI
 * @param[out] eui 8 byte buffer
 * 
 * */
void System_getDevEUI(void *receiver, void *eui);

/** Retrieve the Application Key
 *
 * @param[in] receiver the receiver of this AppKey
 * @param[out] key 16 byte buffer
 * 
 * */
void System_getAppKey(void *receiver, void *key);

void System_getNwkSKey(void *receiver, void *key);
void System_getAppSKey(void *receiver, void *key);

uint32_t System_getDevAddr(void *receiver);

void System_setNwkSKey(void *receiver, const void *key);
void System_setAppSKey(void *receiver, const void *key);
void System_setDevAddr(void *receiver, uint32_t devAddr);

void System_setStatus(void *receiver, uint8_t value);
uint8_t System_getStatus(void *receiver);
    
/** Get the current value of the up counter
 * 
 * */
uint16_t System_getUp(void *receiver);

/** Return the current value of the upcounter and post increment
 * 
 * */
uint16_t System_incrementUp(void *receiver);

/** Reset the up counter 
 * 
 * */
void System_resetUp(void *receiver);

/** Get the last received down counter
 * 
 * */
uint16_t System_getDown(void *receiver);

/** Receive a downcounter value
 * 
 * 
 * */
bool System_receiveDown(void *receiver, uint16_t counter, uint16_t maxGap);

/** Reset teh the down counter
 * 
 * 
 * */
void System_resetDown(void *receiver);

bool System_getChannel(void *receiver, uint8_t chIndex, uint32_t *freq, uint8_t *minRate, uint8_t *maxRate);
bool System_setChannel(void *receiver, uint8_t chIndex, uint32_t freq, uint8_t minRate, uint8_t maxRate);
bool System_maskChannel(void *receiver, uint8_t chIndex);
bool System_unmaskChannel(void *receiver, uint8_t chIndex);
bool System_channelIsMasked(void *receiver, uint8_t chIndex);

uint8_t System_getBatteryLevel(void *receiver);

uint8_t System_getRX1DROffset(void *receiver);
uint8_t System_getMaxDutyCycle(void *receiver);
uint8_t System_getRX1Delay(void *receiver);
uint8_t System_getNbTrans(void *receiver);
uint8_t System_getTXPower(void *receiver);
uint8_t System_getTXRate(void *receiver);
uint32_t System_getRX2Freq(void *receiver);
uint8_t System_getRX2DataRate(void *receiver);

void System_setRX1DROffset(void *receiver, uint8_t value);
void System_setMaxDutyCycle(void *receiver, uint8_t value);
void System_setRX1Delay(void *receiver, uint8_t value);
void System_setTXPower(void *receiver, uint8_t value);
void System_setNbTrans(void *receiver, uint8_t value);
void System_setTXRate(void *receiver, uint8_t value);
void System_setRX2Freq(void *receiver, uint32_t value);
void System_setRX2DataRate(void *receiver, uint8_t value);

void System_logLinkStatus(void *receiver, uint8_t margin, uint8_t gwCount);

uint8_t System_rand(void);

#ifdef __cplusplus
}
#endif

#endif
