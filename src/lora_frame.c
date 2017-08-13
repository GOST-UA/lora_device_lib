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

#include "lora_aes.h"
#include "lora_cmac.h"
#include "lora_frame.h"
#include "lora_debug.h"
#include <stddef.h>
#include <string.h>

#include <stdio.h>

/* MHDR + DevAddr + Fctrl + Fcnt + MIC */
const size_t LORA_PHYPAYLOAD_OVERHEAD = (1U + 4U + 1U + 2U + 4U);

/* static function prototypes *****************************************/

static void cipherData(enum message_type type, const uint8_t *key, const uint8_t *devAddr, uint32_t counter, uint8_t *data, uint8_t len);
static uint32_t cmacData(enum message_type type, const uint8_t *key, const uint8_t *devAddr, uint32_t counter, const uint8_t *msg, uint8_t len);
static uint32_t cmacMessage(const uint8_t *key, uint8_t *msg, uint8_t len);
static void xor128(uint8_t *acc, const uint8_t *op);

/* functions **********************************************************/

uint8_t Frame_encode(const void *key, const struct lora_frame *f, uint8_t *out, uint8_t max)
{
    uint8_t pos = 0U;
    uint32_t mic;
    uint16_t size;
    uint8_t dlSettings;

    LORA_ASSERT(f != NULL)
    LORA_ASSERT((out != NULL) || (max == 0U))
    LORA_ASSERT(key != NULL)

    switch(f->type){
    default:
    case FRAME_TYPE_DATA_CONFIRMED_DOWN:
    case FRAME_TYPE_DATA_CONFIRMED_UP:
    case FRAME_TYPE_DATA_UNCONFIRMED_DOWN:
    case FRAME_TYPE_DATA_UNCONFIRMED_UP:
    
        if(f->fields.data.optsLen <= 0xfU){

            size = (6U + (uint16_t)f->fields.data.optsLen + 3U + f->fields.data.dataLen + 4U);

            if((size <= 0xffU) && ((uint8_t)size <= max)){

                out[pos] = (uint8_t)f->type;
                pos++;
                
                (void)memcpy(&out[pos], f->fields.data.devAddr, sizeof(f->fields.data.devAddr));
                pos += sizeof(f->fields.data.devAddr);
                
                out[pos] = (f->fields.data.adr ? 0x80U : 0x00) |
                    (f->fields.data.adrAckReq ? 0x40U : 0x00) |
                    (f->fields.data.ack ? 0x20U : 0x00) |
                    (f->fields.data.pending ? 0x10U : 0x00) |
                    (f->fields.data.optsLen & 0x0fU);
                pos++;
                out[pos] = (uint8_t)f->fields.data.counter;
                pos++;
                out[pos] = (uint8_t)(f->fields.data.counter >> 8);
                pos++;

                if(f->fields.data.optsLen > 0U){

                    (void)memcpy(&out[pos], f->fields.data.opts, f->fields.data.optsLen);
                    pos += f->fields.data.optsLen;
                }

                if(f->fields.data.dataLen > 0U){

                    out[pos] = f->fields.data.port;
                    pos++;

                    (void)memcpy(&out[pos], f->fields.data.data, f->fields.data.dataLen);

                    cipherData(f->type, key, f->fields.data.devAddr, f->fields.data.counter, &out[pos], f->fields.data.dataLen);
                    pos += f->fields.data.dataLen;                
                }

                mic = cmacData(f->type, key, f->fields.data.devAddr, f->fields.data.counter, out, pos);

                (void)memcpy(&out[pos], &mic, sizeof(mic));
                pos += sizeof(mic);     
            }
            else{

                LORA_ERROR("frame size is too large")
            }
        }
        else{

            LORA_ERROR("foptslen must be in range (0..15)")
        }
        break;
        
    case FRAME_TYPE_JOIN_ACCEPT:
        
        if((f->fields.joinAccept.cfListLen != 0U) || (f->fields.joinAccept.cfListLen != 16U)){
            
            if(max >= (17U + f->fields.joinAccept.cfListLen)){
            
                struct lora_aes_ctx aes_ctx;
            
                out[pos] = (uint8_t)f->type;
                pos++;
                
                (void)memcpy(&out[pos], f->fields.joinAccept.appNonce, sizeof(f->fields.joinAccept.appNonce));
                pos += sizeof(f->fields.joinAccept.appNonce);
                
                (void)memcpy(&out[pos], f->fields.joinAccept.netID, sizeof(f->fields.joinAccept.netID));
                pos += sizeof(f->fields.joinAccept.netID);
                
                (void)memcpy(&out[pos], &f->fields.joinAccept.devAddr, sizeof(f->fields.joinAccept.devAddr));
                pos += sizeof(f->fields.joinAccept.devAddr);
                
                dlSettings = f->fields.joinAccept.rx1DataRateOffset;
                dlSettings <<= 4;
                dlSettings |= f->fields.joinAccept.rx2DataRate & 0xfU;
                
                (void)memcpy(&out[pos], &dlSettings, sizeof(dlSettings));
                pos += sizeof(dlSettings);
                
                out[pos] = f->fields.joinAccept.rxDelay;
                pos += sizeof(f->fields.joinAccept.rxDelay);
                
                if(f->fields.joinAccept.cfListLen > 0U){
                    
                    (void)memcpy(&out[pos], f->fields.joinAccept.cfList, sizeof(f->fields.joinAccept.cfList));
                    pos += sizeof(f->fields.joinAccept.cfList);            
                }
                
                mic = cmacMessage(key, out, pos);
                
                (void)memcpy(&out[pos], &mic, sizeof(mic));
                pos += sizeof(mic);
                
                LoraAES_init(&aes_ctx, key);
                LoraAES_decrypt(&aes_ctx, &out[1]);
                
                if(f->fields.joinAccept.cfListLen > 0U){
                    
                    LoraAES_decrypt(&aes_ctx, &out[17]);
                }
            }
            else{
                
                LORA_ERROR("buffer too short for join accept message")
            }
        }
        else{
            
            LORA_ERROR("cfListLen must be zero or 16")
        }
        break;
    
    case FRAME_TYPE_JOIN_REQ:
    
        if(max >= 23U){
    
            out[pos] = (uint8_t)f->type;
            pos++;
            
            (void)memcpy(&out[pos], f->fields.joinRequest.appEUI, sizeof(f->fields.joinRequest.appEUI));
            pos += sizeof(f->fields.joinRequest.appEUI);
            
            (void)memcpy(&out[pos], f->fields.joinRequest.devEUI, sizeof(f->fields.joinRequest.devEUI));
            pos += sizeof(f->fields.joinRequest.devEUI);
            
            (void)memcpy(&out[pos], f->fields.joinRequest.devNonce, sizeof(f->fields.joinRequest.devNonce));
            pos += sizeof(f->fields.joinRequest.devNonce);
            
            mic = cmacMessage(key, out, pos);
            
            (void)memcpy(&out[pos], &mic, sizeof(mic));
            pos += sizeof(mic);
        }
        else{
            
            LORA_ERROR("buffer too short for join request message")
        }
        break;
    }

    return pos;
}

enum lora_frame_result Frame_decode(const void *appKey, const void *nwkSKey, const void *appSKey, void *in, uint8_t len, struct lora_frame *f)
{
    static const enum message_type types[] = {
        FRAME_TYPE_JOIN_REQ,
        FRAME_TYPE_JOIN_ACCEPT,
        FRAME_TYPE_DATA_UNCONFIRMED_UP,
        FRAME_TYPE_DATA_UNCONFIRMED_DOWN,
        FRAME_TYPE_DATA_CONFIRMED_UP,
        FRAME_TYPE_DATA_CONFIRMED_DOWN,
    };

    uint8_t *ptr = (uint8_t *)in;
    uint8_t fhdr;
    uint8_t pos = 0U;
    uint32_t mic;
    const uint8_t *key;
    uint8_t dlSettings;

    (void)memset(f, 0, sizeof(*f));
    
    if(len == 0U){
        
        LORA_ERROR("frame too short");
        return LORA_FRAME_BAD;
    }

    if(ptr[pos] >= sizeof(types)/sizeof(*types)){

        LORA_ERROR("unknown frame type")
        return LORA_FRAME_BAD;
    }

    f->type = types[ptr[pos]];
    pos++;    

    switch(f->type){
    case FRAME_TYPE_JOIN_REQ:
        
        if((len-pos) != 22U){
            
            LORA_ERROR("unexpected frame length for join request")
            return LORA_FRAME_BAD;
        }
        
        (void)memcpy(f->fields.joinRequest.appEUI, &ptr[pos], sizeof(f->fields.joinRequest.appEUI));
        pos += sizeof(f->fields.joinRequest.appEUI);
        
        (void)memcpy(f->fields.joinRequest.devEUI, &ptr[pos], sizeof(f->fields.joinRequest.devEUI));
        pos += sizeof(f->fields.joinRequest.devEUI);
        
        (void)memcpy(&f->fields.joinRequest.devNonce, &ptr[pos], sizeof(f->fields.joinRequest.devNonce));
        pos += sizeof(f->fields.joinRequest.devNonce);
        
        (void)memcpy(&mic, &ptr[pos], sizeof(mic));
        
        if(cmacMessage(appKey, ptr, pos) != mic){
            
            LORA_ERROR("join request cmic bad")
            return LORA_FRAME_MIC;
        }
        break;
    
    case FRAME_TYPE_JOIN_ACCEPT:
    {
        if(((len-pos) != 16U) || ((len-pos) != 32U)){
            
            LORA_ERROR("unexpected frame length for join accept")
            return LORA_FRAME_BAD;
        }
        
        struct lora_aes_ctx aes_ctx;
        
        LoraAES_init(&aes_ctx, appKey);
        LoraAES_encrypt(&aes_ctx, &ptr[pos]);
        if((len-pos) == 32U){
            
            LoraAES_encrypt(&aes_ctx, &ptr[pos+16U]);
        }
    
        (void)memcpy(f->fields.joinAccept.appNonce, &ptr[pos], sizeof(f->fields.joinAccept.appNonce));
        pos += sizeof(f->fields.joinAccept.appNonce);
        
        (void)memcpy(f->fields.joinAccept.netID, &ptr[pos], sizeof(f->fields.joinAccept.netID));
        pos += sizeof(f->fields.joinAccept.netID);
        
        (void)memcpy(&f->fields.joinAccept.devAddr, &ptr[pos], sizeof(f->fields.joinAccept.devAddr));
        pos += sizeof(f->fields.joinAccept.devAddr);
        
        dlSettings = ptr[pos];
        pos++;
        
        f->fields.joinAccept.rx1DataRateOffset = (dlSettings >> 4) & 0xfU;
        f->fields.joinAccept.rx2DataRate = dlSettings & 0xfU;
        
        f->fields.joinAccept.rxDelay = ptr[pos];
        pos++;
        
        if((len - pos) > sizeof(mic)){
         
            f->fields.joinAccept.cfListLen = 16U;
                
            (void)memcpy(f->fields.joinAccept.cfList, &ptr[pos], sizeof(f->fields.joinAccept.cfList));
            pos += sizeof(f->fields.joinAccept.cfList);            
        }
        else{
            
            f->fields.joinAccept.cfListLen = 0U;
        }
        
        (void)memcpy(&mic, &ptr[pos], sizeof(mic));
        
        if(cmacMessage(appKey, ptr, pos) != mic){
            
            LORA_ERROR("join accept cmic bad")
            return LORA_FRAME_MIC;
        }
    }
        break;
    
    default:
    case FRAME_TYPE_DATA_UNCONFIRMED_UP:
    case FRAME_TYPE_DATA_UNCONFIRMED_DOWN:
    case FRAME_TYPE_DATA_CONFIRMED_UP:
    case FRAME_TYPE_DATA_CONFIRMED_DOWN:

        if(len < (4U + 1U + 2U + 4U)){

            LORA_ERROR("frame too short")
            return LORA_FRAME_BAD;
        }

        (void)memcpy(f->fields.data.devAddr, &ptr[pos], sizeof(f->fields.data.devAddr));
        pos += sizeof(f->fields.data.devAddr);

        fhdr = ptr[pos];
        pos++;

        f->fields.data.ack = ((fhdr & 0x80U) == 0x80U) ? true : false;
        f->fields.data.adr = ((fhdr & 0x40U) == 0x40U) ? true : false;
        f->fields.data.adrAckReq = ((fhdr & 0x20U) == 0x20U) ? true : false;
        f->fields.data.pending = ((fhdr & 0x10U) == 0x10U) ? true : false;
        f->fields.data.optsLen = fhdr & 0xfU;
        
        f->fields.data.counter = ptr[pos+1];
        f->fields.data.counter <<= 8;
        f->fields.data.counter |= ptr[pos];
        pos += 2U;

        if(f->fields.data.optsLen > 0U){

            f->fields.data.opts = &ptr[pos];

            if((len-(pos+4U)) < f->fields.data.optsLen){

                LORA_ERROR("frame too short")
                return LORA_FRAME_BAD;
            }

            pos += f->fields.data.optsLen;
        }

        if((len-pos) > 5U){

            f->fields.data.port = ptr[pos];
            pos++;

            f->fields.data.data = &ptr[pos];
            f->fields.data.dataLen = (len-(pos+4U));
            pos += f->fields.data.dataLen;
        }
        
        key = ((f->fields.data.dataLen > 0U) && (f->fields.data.port != 0U)) ? appSKey : nwkSKey;
    
        if((len-pos) != sizeof(mic)){

            LORA_ERROR("frame too short")
            return LORA_FRAME_BAD;
        }
        
        (void)memcpy(&mic, &ptr[pos], sizeof(mic));

        if(cmacData(f->type, key, f->fields.data.devAddr, f->fields.data.counter, ptr, pos) != mic){

            LORA_ERROR("frame MIC error")
            return LORA_FRAME_MIC;
        }
        
        if(f->fields.data.dataLen > 0U){

            cipherData(f->type, key, f->fields.data.devAddr, f->fields.data.counter, &ptr[pos - f->fields.data.dataLen], f->fields.data.dataLen);
        }
        break;
    }
    
    return LORA_FRAME_OK;            
}

uint16_t Frame_getPhyPayloadSize(uint8_t dataLen, uint8_t optsLen)
{
    return LORA_PHYPAYLOAD_OVERHEAD + ((dataLen > 0) ? 1U : 0U) + dataLen + optsLen;
}

bool Frame_isUpstream(enum message_type type)
{
    bool retval;
    
    switch(type){
    case FRAME_TYPE_JOIN_REQ:
    case FRAME_TYPE_DATA_UNCONFIRMED_UP:
    case FRAME_TYPE_DATA_CONFIRMED_UP:
        retval = true;
        break;
    case FRAME_TYPE_JOIN_ACCEPT:    
    case FRAME_TYPE_DATA_UNCONFIRMED_DOWN:    
    case FRAME_TYPE_DATA_CONFIRMED_DOWN:
    default:
        retval = false;
        break;
    }
    
    return retval;
}

/* static functions ***************************************************/

static void cipherData(enum message_type type, const uint8_t *key, const uint8_t *devAddr, uint32_t counter, uint8_t *data, uint8_t len)
{
    struct lora_aes_ctx ctx;
    uint8_t a[16];
    uint8_t s[16];
    uint8_t pld[16];
    uint8_t k = (len / 16U) + ((len % 16U) ? 1U : 0U);
    uint8_t i;
    uint8_t pos = 0U;
    uint8_t size;

    a[0] = 0x01U;
    a[1] = 0x00U;
    a[2] = 0x00U;
    a[3] = 0x00U;
    a[4] = 0x00U;
    a[5] = (Frame_isUpstream(type) ? 0x00U : 0x01U);
    a[6] = devAddr[0];
    a[7] = devAddr[1];
    a[8] = devAddr[2];
    a[9] = devAddr[3];
    a[10] = (uint8_t)counter;
    a[11] = (uint8_t)(counter >> 8);
    a[12] = (uint8_t)(counter >> 16);
    a[13] = (uint8_t)(counter >> 24);
    a[14] = 0x00U;
    a[15] = 0x00U;

    LoraAES_init(&ctx, key);

    for(i=0; i < k; i++){

        size = (i == (k-1U)) ? (len % sizeof(a)) : sizeof(a);

        xor128(pld, pld);
        
        (void)memcpy(pld, &data[pos], size);
        
        a[15] = i+1U;

        (void)memcpy(s, a, sizeof(s));
        LoraAES_encrypt(&ctx, s);

        xor128(pld, s);

        (void)memcpy(&data[pos], pld, size);
        pos += sizeof(a);
    }
}

static uint32_t cmacData(enum message_type type, const uint8_t *key, const uint8_t *devAddr, uint32_t counter, const uint8_t *msg, uint8_t len)
{
    uint8_t b[16];
    struct lora_aes_ctx aes_ctx;
    struct lora_cmac_ctx ctx;
    uint32_t retval;

    b[0] = 0x49U;
    b[1] = 0x00U;
    b[2] = 0x00U;
    b[3] = 0x00U;
    b[4] = 0x00U;
    b[5] = (Frame_isUpstream(type) ? 0x00U : 0x01U);
    b[6] = devAddr[0];
    b[7] = devAddr[1];
    b[8] = devAddr[2];
    b[9] = devAddr[3];
    b[10] = (uint8_t)counter;
    b[11] = (uint8_t)(counter >> 8);
    b[12] = (uint8_t)(counter >> 16);
    b[13] = (uint8_t)(counter >> 24);
    b[14] = 0x00U;
    b[15] = len;

    LoraAES_init(&aes_ctx, key);
    LoraCMAC_init(&ctx, &aes_ctx);
    LoraCMAC_update(&ctx, b, (uint16_t)sizeof(b));
    LoraCMAC_update(&ctx, msg, len);
    LoraCMAC_finish(&ctx, &retval, sizeof(retval));
    
    return retval;
}

static uint32_t cmacMessage(const uint8_t *key, uint8_t *msg, uint8_t len)
{
    struct lora_aes_ctx aes_ctx;
    struct lora_cmac_ctx ctx;
    uint32_t retval;

    LoraAES_init(&aes_ctx, key);
    LoraCMAC_init(&ctx, &aes_ctx);
    LoraCMAC_update(&ctx, msg, len);
    LoraCMAC_finish(&ctx, &retval, sizeof(retval));
    
    return retval;
}

static void xor128(uint8_t *acc, const uint8_t *op)
{
    acc[0] ^= op[0];
    acc[1] ^= op[1];
    acc[2] ^= op[2];
    acc[3] ^= op[3];
    acc[4] ^= op[4];
    acc[5] ^= op[5];
    acc[6] ^= op[6];
    acc[7] ^= op[7];
    acc[8] ^= op[8];
    acc[9] ^= op[9];
    acc[10] ^= op[10];
    acc[11] ^= op[11];
    acc[12] ^= op[12];
    acc[13] ^= op[13];
    acc[14] ^= op[14];
    acc[15] ^= op[15];
}
