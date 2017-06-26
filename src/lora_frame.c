#include "lora_aes.h"
#include "lora_cmac.h"
#include "lora_frame.h"
#include "lora_debug.h"
#include <stddef.h>
#include <string.h>

/* MHDR + DevAddr + Fctrl + Fcnt + MIC */
const size_t LORA_PHYPAYLOAD_OVERHEAD = (1U + 4U + 1U + 2U + 4U);

/* static function prototypes *****************************************/

static void cipher(enum message_type type, const uint8_t *key, uint32_t devAddr, uint32_t counter, uint8_t *data, uint8_t len);
static uint32_t cmac(enum message_type type, const uint8_t *key, uint32_t devAddr, uint32_t counter, const uint8_t *msg, uint8_t len);
static void xor128(uint8_t *acc, const uint8_t *op);

/* functions **********************************************************/

uint8_t LoraFrame_encode(const uint8_t *key, const struct lora_frame *f, uint8_t *out, uint8_t max)
{
    uint8_t pos = 0U;
    uint32_t mic;
    uint16_t size;

    if(f->optsLen <= 0xfU){

        size = (6U + (uint16_t)f->optsLen + 3U + f->dataLen + 4U);

        if((size <= 0xffU) && ((uint8_t)size <= max)){

            out[pos] = (uint8_t)f->type;
            pos++;
            out[pos] = (uint8_t)f->devAddr;
            pos++;
            out[pos] = (uint8_t)(f->devAddr >> 8);
            pos++;
            out[pos] = (uint8_t)(f->devAddr >> 16);
            pos++;
            out[pos] = (uint8_t)(f->devAddr >> 24);
            pos++;
            out[pos] = (f->adr ? 0x80U : 0x00) |
                (f->adrAckReq ? 0x40U : 0x00) |
                (f->ack ? 0x20U : 0x00) |
                (f->pending ? 0x10U : 0x00) |
                (f->optsLen & 0x0fU);
            pos++;
            out[pos] = (uint8_t)f->counter;
            pos++;
            out[pos] = (uint8_t)(f->counter >> 8);
            pos++;

            if(f->optsLen > 0U){

                (void)memcpy(&out[pos], f->opts, f->optsLen);
                pos += f->optsLen;
            }

            if(f->dataLen > 0U){

                out[pos] = f->port;
                pos++;

                (void)memcpy(&out[pos], f->data, f->dataLen);

                cipher(f->type, key, f->devAddr, f->counter, &out[pos], f->dataLen);
                pos += f->dataLen;                
            }

            mic = cmac(f->type, key, f->devAddr, f->counter, out, pos);

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

    return pos;
}

enum lora_frame_result LoraFrame_decode(const uint8_t *netKey, const uint8_t *appKey, uint8_t *in, uint8_t len, struct lora_frame *f)
{
    const enum message_type types[] = {
        FRAME_TYPE_JOIN_REQ,
        FRAME_TYPE_JOIN_ACCEPT,
        FRAME_TYPE_DATA_UNCONFIRMED_UP,
        FRAME_TYPE_DATA_UNCONFIRMED_DOWN,
        FRAME_TYPE_DATA_CONFIRMED_UP,
        FRAME_TYPE_DATA_CONFIRMED_DOWN,
    };

    uint8_t fhdr;
    uint8_t pos = 0U;
    uint32_t mic;
    const uint8_t *key;

    (void)memset(f, 0, sizeof(*f));
    
    if(len < (6U + 3U + 4U)){

        LORA_ERROR("frame too short")
        return LORA_FRAME_BAD;
    }

    if(in[pos] >= sizeof(types)/sizeof(types)){

        LORA_ERROR("unknown frame type")
        return LORA_FRAME_BAD;
    }

    f->type = types[in[pos]];
    pos++;

    f->devAddr = in[pos+3];
    f->devAddr <<= 8;
    f->devAddr |= in[pos+2];
    f->devAddr <<= 8;
    f->devAddr |= in[pos+1];
    f->devAddr <<= 8;
    f->devAddr |= in[pos];
    pos += 4U;

    fhdr = in[pos];
    pos++;

    f->ack = ((fhdr & 0x80U) == 0x80U) ? true : false;
    f->adr = ((fhdr & 0x40U) == 0x40U) ? true : false;
    f->adrAckReq = ((fhdr & 0x20U) == 0x20U) ? true : false;
    f->pending = ((fhdr & 0x10U) == 0x10U) ? true : false;
    f->optsLen = fhdr & 0xfU;
    
    f->counter = 0U;
    f->counter = in[pos+1];
    f->counter <<= 8;
    f->counter |= in[pos];
    pos += 2U;

    if(f->optsLen > 0U){

        f->opts = &in[pos];

        if((len-(pos+4U)) < f->optsLen){

            LORA_ERROR("frame too short")
            return LORA_FRAME_BAD;
        }

        pos += f->optsLen;
    }

    if((len-pos) > 5U){

        f->port = in[pos];
        pos++;

        f->data = &in[pos];
        f->dataLen = (len-(pos+4U));
        pos += f->dataLen;
    }

    if((len-pos) != sizeof(mic)){

        LORA_ERROR("frame too short")
        return LORA_FRAME_BAD;
    }

    key = ((f->dataLen > 0U) && (f->port != 0U)) ? appKey : netKey;

    (void)memcpy(&mic, &in[pos], sizeof(mic));

    if(cmac(f->type, key, f->devAddr, f->counter, in, pos) != mic){

        LORA_ERROR("frame MIC error")
        return LORA_FRAME_MIC;
    }

    if(f->dataLen > 0U){

        cipher(f->type, key, f->devAddr, f->counter, &in[pos - f->dataLen], f->dataLen);
    }

    

    return LORA_FRAME_OK;            
}

uint16_t LoraFrame_getPhyPayloadSize(uint8_t dataLen, uint8_t optsLen)
{
    return LORA_PHYPAYLOAD_OVERHEAD + ((dataLen > 0) ? 1U : 0U) + dataLen + optsLen;
}

bool LoraFrame_isUpstream(enum message_type type)
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

static void cipher(enum message_type type, const uint8_t *key, uint32_t devAddr, uint32_t counter, uint8_t *data, uint8_t len)
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
    a[5] = (LoraFrame_isUpstream(type) ? 0x00U : 0x01U);
    a[6] = (uint8_t)devAddr;
    a[7] = (uint8_t)(devAddr >> 8);
    a[8] = (uint8_t)(devAddr >> 16);
    a[9] = (uint8_t)(devAddr >> 24);    
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

static uint32_t cmac(enum message_type type, const uint8_t *key, uint32_t devAddr, uint32_t counter, const uint8_t *msg, uint8_t len)
{
    uint8_t b[16];
    struct lora_aes_ctx aes_ctx;
    struct lora_cmac_ctx ctx;

    b[0] = 0x49U;
    b[1] = 0x00U;
    b[2] = 0x00U;
    b[3] = 0x00U;
    b[4] = 0x00U;
    b[5] = (LoraFrame_isUpstream(type) ? 0x00U : 0x01U);
    b[6] = (uint8_t)devAddr;
    b[7] = (uint8_t)(devAddr >> 8);
    b[8] = (uint8_t)(devAddr >> 16);
    b[9] = (uint8_t)(devAddr >> 24);
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
    return LoraCMAC_finish(&ctx);
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
