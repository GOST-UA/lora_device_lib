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

#ifndef LORA_SYSTEM_H
#define LORA_SYSTEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/** Get system time (ticks)
 * 
 * @return system time (ticks)
 * 
 * */
uint64_t System_time(void);

/** Set the value of receiver to value in an atomic operation
 * 
 * @param[out] receiver
 * @param[in] value
 * 
 * */
void System_atomic_setPtr(void **receiver, void *value);

/** Get AppEUI
 * 
 * @param[in] receiver system object
 * @param[out] value buffer of at least 8 bytes
 * 
 * */
void System_getAppEUI(void *receiver, void *value);

/** Get DevEUI
 *
 * @param[in] receiver system object
 * @param[out] value buffer of at least 8 bytes
 * 
 * */
void System_getDevEUI(void *receiver, void *value);

/** Get AppKey
 *
 * @param[in] receiver system object
 * @param[out] value buffer of at least 16 bytes
 * 
 * */
void System_getAppKey(void *receiver, void *value);

/** Get NwkSKey
 *
 * @param[in] receiver system object
 * @param[out] value buffer of at least 16 bytes
 * 
 * */
void System_getNwkSKey(void *receiver, void *value);

/** Get AppSKey
 *
 * @param[in] receiver system object
 * @param[out] value buffer of at least 16 bytes
 * 
 * */
void System_getAppSKey(void *receiver, void *value);

/** Get DevAddr
 *
 * @param[in] receiver system object
 * @return Device Address
 * 
 * */
uint32_t System_getDevAddr(void *receiver);

/** Store NwkSKey
 * 
 * @param[in] receiver system object
 * @param[in] value 16 byte field
 * 
 * */
void System_setNwkSKey(void *receiver, const void *value);

/** Store AppSKey
 * 
 * @param[in] receiver system object
 * @param[in] value 16 byte field
 * 
 * */
void System_setAppSKey(void *receiver, const void *value);

/** Store DevAddr
 * 
 * @param[in] receiver system object
 * @param[in] value Device Address
 * 
 * */
void System_setDevAddr(void *receiver, uint32_t value);

/** Store MAC status
 * 
 * @param[in] receiver system object
 * @param[in] value status
 * 
 * */
void System_setStatus(void *receiver, uint8_t value);

/** Get MAC status
 * 
 * @param[in] recevier system object
 * @return status
 * 
 * */
uint8_t System_getStatus(void *receiver);
    
/** Get up counter value
 * 
 * @param[in] receiver system object
 * @return up counter value
 * 
 * */
uint16_t System_getUp(void *receiver);

/** Get up counter value (and increment the stored up counter value)
 * 
 * Equivalent functionality to:
 * 
 * @code
 * 
 * uint16_t retval = System_getUp(NULL);
 * (void)System_incrementUp(NULL);
 * return retval;
 *  
 * @endcode
 * 
 * 
 * @param[in] receiver system object
 * @return up counter value (before increment)
 * 
 * */
uint16_t System_incrementUp(void *receiver);

/** Reset up counter to zero
 * 
 * @param[in] receiver system object
 * 
 * */
void System_resetUp(void *receiver);

/** Get down counter value
 * 
 * @param[in] receiver system object
 * 
 * */
uint16_t System_getDown(void *receiver);

/** Receive a down counter value
 * 
 * @note if successful `counter` will become the new stored down counter value
 * 
 * @param[in] receiver system object
 * @param[in] counter down counter value to receive
 * @param[in] maxGap the maximum acceptible difference between stored down counter and the counter argument
 * @return true if counter was no more than (maxGap + System_getDown)
 * 
 * */
bool System_receiveDown(void *receiver, uint16_t counter, uint16_t maxGap);

/** Reset the the down counter to zero
 * 
 * @param[in] receiver system object
 * 
 * */
void System_resetDown(void *receiver);

/** Get a stored channel configuration
 * 
 * @param[in] receiver system object
 * @param[in] chIndex channel index (from zero)
 * @param[out] freq frequency in Hz
 * @param[out] minRate minimum rate allowed on this channel
 * @param[out] minRate maximum rate allowed on this channel
 * 
 * @return true if chIndex is within bounds
 * 
 * */
bool System_getChannel(void *receiver, uint8_t chIndex, uint32_t *freq, uint8_t *minRate, uint8_t *maxRate);

/** Store a channel configuration
 * 
 * @param[in] receiver system object
 * @param[in] chIndex channel index (from zero)
 * @param[in] freq frequency in Hz
 * @param[in] minRate minimum rate allowed on this channel
 * @param[in] minRate maximum rate allowed on this channel
 * 
 * @return true if chIndex is within bounds
 * 
 * */
bool System_setChannel(void *receiver, uint8_t chIndex, uint32_t freq, uint8_t minRate, uint8_t maxRate);

/** Mask a channel
 * 
 * @param[in] receiver system object
 * @param[in] chIndex channel index (from zero)
 * 
 * @return true if chIndex is within bounds
 * 
 * 
 * */
bool System_maskChannel(void *receiver, uint8_t chIndex);

/** Unmask a channel
 * 
 * @param[in] receiver system object
 * @param[in] chIndex channel index (from zero)
 * 
 * @return true if chIndex is within bounds
 * 
 * */
bool System_unmaskChannel(void *receiver, uint8_t chIndex);


/** Test if a channel is masked
 * 
 * @note out of bounds chIndex must indicate 'masked'
 * 
 * @param[in] receiver system object
 * @param[in] chIndex channel index (from zero)
 * 
 * @return true if chIndex is masked
 * 
 * */
bool System_channelIsMasked(void *receiver, uint8_t chIndex);

/** 
 * @param[in] receiver system object
 * @return battery level
 * 
 * */
uint8_t System_getBatteryLevel(void *receiver);

/** 
 * @param[in] receiver system object
 * @return RX1DROffset
 * 
 * */
uint8_t System_getRX1DROffset(void *receiver);

/** 
 * @param[in] receiver system object
 * @return MaxDutyCycle
 * 
 * */
uint8_t System_getMaxDutyCycle(void *receiver);

/** 
 * @param[in] receiver system object
 * @return RX1Delay
 * 
 * */
uint8_t System_getRX1Delay(void *receiver);

/** 
 * @param[in] receiver system object
 * @return NbTrans
 * 
 * */
uint8_t System_getNbTrans(void *receiver);

/** 
 * @param[in] receiver system object
 * @return power setting for transmit
 * 
 * */
uint8_t System_getTXPower(void *receiver);

/** 
 * @param[in] receiver system object
 * @return rate setting for transmit
 * 
 * */
uint8_t System_getTXRate(void *receiver);

/** 
 * @param[in] receiver system object
 * @return RX2Freq
 * 
 * */
uint32_t System_getRX2Freq(void *receiver);

/** 
 * @param[in] receiver system object
 * @return RX2DataRate
 * 
 * */
uint8_t System_getRX2DataRate(void *receiver);

/** Store RX1DROffset
 * 
 * @param[in] receiver system object
 * @param[in] value
 * 
 * */
void System_setRX1DROffset(void *receiver, uint8_t value);

/** Store MaxDutyCycle
 * 
 * @param[in] receiver system object
 * @param[in] value
 * 
 * */
void System_setMaxDutyCycle(void *receiver, uint8_t value);

/** Store RXDelay
 * 
 * @param[in] receiver system object
 * @param[in] value
 * 
 * */
void System_setRX1Delay(void *receiver, uint8_t value);

/** Store transmit power setting 
 * 
 * @param[in] receiver system object
 * @param[in] value
 * 
 * */
void System_setTXPower(void *receiver, uint8_t value);

/** Store NbTrans
 * 
 * @param[in] receiver system object
 * @param[in] value
 * 
 * */
void System_setNbTrans(void *receiver, uint8_t value);

/** Store transmit rate setting
 * 
 * @param[in] receiver system object
 * @param[in] value
 * 
 * */
void System_setTXRate(void *receiver, uint8_t value);

/** Store RX2Freq
 * 
 * @param[in] receiver system object
 * @param[in] value
 * 
 * */
void System_setRX2Freq(void *receiver, uint32_t value);

/** Store RX2DataRate
 * 
 * @param[in] receiver system object
 * @param[in] value
 * 
 * */
void System_setRX2DataRate(void *receiver, uint8_t value);

/** Store LinkStatus
 * 
 * @param[in] receiver system object
 * @param[in] value
 * 
 * */
void System_setLinkStatus(void *receiver, uint8_t margin, uint8_t gwCount);

/** Get a random number in range 0..255
 * 
 * @param[in] receiver system object
 * @return random number in range 0..255
 * 
 * */
uint8_t System_rand(void);

#ifdef __cplusplus
}
#endif

#endif
