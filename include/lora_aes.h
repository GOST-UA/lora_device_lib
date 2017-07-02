/* Copyright (c) 2013-2016 Cameron Harper
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


#ifndef LORA_AES_H
#define LORA_AES_H

 /**
 * @defgroup lora_aes default AES block cipher
 * @ingroup lora
 * 
 * Interface to the block cipher defined in FIPS-197
 *
 * @{
 * */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#if defined(LORA_USE_PLATFORM_AES)

struct lora_aes_ctx;

#else

/** Stores the expanded key */
struct lora_aes_ctx {

    uint8_t k[240U];    /**< expanded key */
    uint8_t r;          /**< number of rounds */
};

#endif

#ifndef AES_BLOCK_SIZE
    #define AES_BLOCK_SIZE 16U
#endif

/**
 * Initialise an AES block cipher by expanding a key
 *
 * @param[in] ctx expanded key
 * @param[in] key pointer to 16 byte key
 * 
 * */
void LoraAES_init(struct lora_aes_ctx *ctx, const uint8_t *key);

/**
 * Encrypt a block of memory called state
 *
 * @param[in] ctx
 * @param[in] s pointer to 16 bytes of state (any alignment)
 * 
 * */
void LoraAES_encrypt(const struct lora_aes_ctx *ctx, uint8_t *s);

/**
 * Decrypt a block of memory called state
 * 
 * @param[in] ctx
 * @param[in] s pointer to 16 bytes of state (any alignment)
 * 
 * */
void LoraAES_decrypt(const struct lora_aes_ctx *ctx, uint8_t *s);

#ifdef __cplusplus
}
#endif

/** @} */
#endif
