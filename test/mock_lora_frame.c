#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"

#include "lora_frame.h"

uint8_t LoraFrame_encode(const uint8_t *key, const struct lora_frame *f, uint8_t *out, uint8_t max)
{
    return mock();
}

enum lora_frame_result LoraFrame_decode(const uint8_t *appKey, const uint8_t *netSKey, const uint8_t *appSKey, uint8_t *in, uint8_t len, struct lora_frame *f)
{
    return mock();
}

uint16_t LoraFrame_getPhyPayloadSize(uint8_t dataLen, uint8_t optsLen)
{
    return mock();
}

bool LoraFrame_isUpstream(enum message_type type)
{
    return mock();
}
