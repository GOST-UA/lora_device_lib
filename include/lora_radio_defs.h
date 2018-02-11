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

#ifndef LORA_RADIO_DEFS_H
#define LORA_RADIO_DEFS_H

/** Spreading Factor (aka symbol rate)     
 *
 * */
enum lora_spreading_factor {
    SF_7 = 7,       /**< 128 chips/symbol */
    SF_8,       /**< 256 chips/symbol */
    SF_9,       /**< 512 chips/symbol */
    SF_10,      /**< 1024 chips/symbol */
    SF_11,      /**< 2048 chips/symbol */
    SF_12,      /**< 4096 chips/symbol */
    SF_FSK      /**< NA */
};

/** signal bandwidth */
enum lora_signal_bandwidth {
    BW_125 = 125000, /**< 125 KHz */
    BW_250 = 250000, /**< 250 KHz */
    BW_500 = 500000, /**< 500 KHz */
    BW_FSK
};

enum lora_coding_rate {
    CR_5 = 1,   /**< 4/5 */
    CR_6,       /**< 4/6 */
    CR_7,       /**< 4/7 */
    CR_8,       /**< 4/8 */
};

#endif
