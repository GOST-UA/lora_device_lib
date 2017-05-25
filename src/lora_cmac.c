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

/* includes ***********************************************************/

#if !defined(LORA_USE_PLATFORM_CMAC)

#include "lora_aes.h"
#include "lora_cmac.h"
#include "lora_debug.h"

#include <string.h>
#include <stdio.h>

/* defines ************************************************************/

typedef uint8_t word_t;

#define WORD_BLOCK_SIZE (AES_BLOCK_SIZE / sizeof(word_t))
#define LSB 0x01U
#define MSB 0x80U

/* static function prototypes *****************************************/

/**
 * XOR an aligned AES block (may be aliased)
 *
 * @param[out] acc accumulator
 * @param[in] mask XORed with accumulator
 *
 * */
static void xor128(word_t *acc, const word_t *mask);

/**
 * left shift (by one bit) a 128bit vector
 *
 * @param[in/out] v vector to shift
 *
 * */
static void leftShift128(word_t *v);

/* functions  *********************************************************/

void LoraCMAC_init(struct lora_cmac_ctx *ctx, const struct lora_aes_ctx *aes_ctx)
{    
    LORA_ASSERT(ctx != NULL)
    LORA_ASSERT(aes_ctx != NULL)

    (void)memset(ctx, 0, sizeof(*ctx));
    ctx->aes_ctx = aes_ctx;
}

void LoraCMAC_update(struct lora_cmac_ctx *ctx, const void *data, uint8_t len)
{
    LORA_ASSERT(ctx != NULL)
    LORA_ASSERT((len == 0U) || (data != NULL))

    size_t part = ctx->size % sizeof(ctx->m);
    size_t i;
    size_t blocks;
    size_t pos = 0U;
    const uint8_t *in = (const uint8_t *)data;
    
    if((len > 0U) && ((ctx->size + len) > ctx->size)){

        if((part + len) > sizeof(ctx->m)){

            blocks = (part + len) / sizeof(ctx->m);

            if(((part + len) % sizeof(ctx->m)) == 0U){

                blocks--;
            }

            (void)memcpy(&ctx->m[part], in, sizeof(ctx->m) - part);
            pos += sizeof(ctx->m) - part;

            for(i=0U; i < blocks; i++){

                xor128(ctx->m, ctx->k);
                LoraAES_encrypt(ctx->aes_ctx, ctx->m);
                (void)memcpy(ctx->k, ctx->m, sizeof(ctx->m));
                
                if(i < (blocks-1U)){

                    (void)memcpy(ctx->m, &in[pos], sizeof(ctx->m));
                    pos += sizeof(ctx->m);
                } 
            }

            part = 0U;   
        }

        (void)memcpy(&ctx->m[part], &in[pos], len - pos);
        ctx->size += len;
    }
}

uint32_t LoraCMAC_finish(struct lora_cmac_ctx *ctx)
{
    LORA_ASSERT(ctx != NULL)

    uint32_t retval;
    word_t k1[WORD_BLOCK_SIZE];
    word_t k2[WORD_BLOCK_SIZE];
    word_t k[WORD_BLOCK_SIZE];
    word_t m[WORD_BLOCK_SIZE];

    (void)memset(k, 0, sizeof(k));

    LoraAES_encrypt(ctx->aes_ctx, k);
    
    (void)memcpy(k1, k, sizeof(k1));
    leftShift128(k1);

    if((*(uint8_t *)k & 0x80U) == 0x80U){

        ((uint8_t *)k1)[AES_BLOCK_SIZE - 1U] ^= 0x87U;
    }

    (void)memcpy(k2, k1, sizeof(k2));
    leftShift128(k2);

    if((*(uint8_t *)k1 & 0x80U) == 0x80U){

        ((uint8_t *)k2)[AES_BLOCK_SIZE - 1U] ^= 0x87U;
    }

    size_t part = ctx->size % sizeof(ctx->m);

    if(ctx->size == 0U){

        (void)memset(m, 0, sizeof(m));
    }
    else if(part > 0){

        (void)memcpy(m, ctx->m, part);
    }
    else{

        (void)memcpy(m, ctx->m, sizeof(ctx->m));
    }
        
    if((ctx->size == 0) || (part > 0U)){

        ((uint8_t *)m)[part] = 0x80U;
        xor128(m, k2);        
    }
    else{

        xor128(m, k1);
    }

    xor128(m, ctx->k);

    LoraAES_encrypt(ctx->aes_ctx, (uint8_t *)m);

    (void)memcpy(&retval, m, sizeof(retval));

    return retval;
}

/* static functions  **************************************************/

static void leftShift128(word_t *v)
{
    word_t t;
    word_t tt;
    word_t carry;
    uint8_t i;
    
    carry = 0U;

    for(i=(uint8_t)WORD_BLOCK_SIZE; i > 0U; i--){

        t = v[i-1U];

        tt = t;
        tt <<= 1;
        tt |= carry;

        carry = ((t & MSB) == MSB) ? LSB : 0x0U;
        v[i-1U] = tt;
    }
}

static void xor128(word_t *acc, const word_t *mask)
{
    uint8_t i;
    
    for(i=0U; i < WORD_BLOCK_SIZE; i++){

        acc[i] ^= mask[i];
    }
}

#endif
