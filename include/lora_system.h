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

void System_getNwkSKey(void *owner, uint8_t *key);
void System_getAppSKey(void *owner, uint8_t *key);

uint32_t System_getDevAddr(void *owner);

void System_setNwkSKey(void *owner, const uint8_t *key);
void System_setAppSKey(void *owner, const uint8_t *key);

void System_setDevAddr(void *owner, uint32_t devAddr);

/** Get the current value of the up counter
 * 
 * */
uint16_t System_getUp(void *owner);

/** Return the current value of the upcounter and post increment
 * 
 * */
uint16_t System_incrementUp(void *owner);

/** Reset the up counter 
 * 
 * */
void System_resetUp(void *owner);

/** Get the last received down counter
 * 
 * */
uint16_t System_getDown(void *owner);

/** Receive a downcounter value
 * 
 * 
 * */
bool System_receiveDown(void *owner, uint16_t counter, uint16_t maxGap);

/** Reset teh the down counter
 * 
 * 
 * */
void System_resetDown(void *owner);

bool System_getChannel(void *owner, uint8_t chIndex, uint32_t *freq, uint8_t *minRate, uint8_t *maxRate);
bool System_setChannel(void *owner, uint8_t chIndex, uint32_t freq, uint8_t minRate, uint8_t maxRate);
bool System_maskChannel(void *owner, uint8_t chIndex);
bool System_unmaskChannel(void *owner, uint8_t chIndex);
bool System_channelIsMasked(void *owner, uint8_t chIndex);

uint8_t System_getBatteryLevel(void *owner);

uint8_t System_getRX1DROffset(void *owner);
uint8_t System_getMaxDutyCycle(void *owner);
uint8_t System_getRX1Delay(void *owner);
uint8_t System_getNbTrans(void *owner);
uint8_t System_getTXPower(void *owner);
uint8_t System_getTXRate(void *owner);
uint32_t System_getRX2Freq(void *owner);
uint8_t System_getRX2DataRate(void *owner);

bool System_setRX1DROffset(void *owner, uint8_t value);
bool System_setMaxDutyCycle(void *owner, uint8_t value);
bool System_setRX1Delay(void *owner, uint8_t value);
bool System_setTXPower(void *owner, uint8_t value);
bool System_setNbTrans(void *owner, uint8_t value);
bool System_setTXRate(void *owner, uint8_t value);
bool System_setRX2Freq(void *owner, uint32_t value);
bool System_setRX2DataRate(void *owner, uint8_t value);


void System_logLinkStatus(void *owner, uint8_t margin, uint8_t gwCount);

uint8_t System_rand(void);

#ifdef __cplusplus
}
#endif

#endif
