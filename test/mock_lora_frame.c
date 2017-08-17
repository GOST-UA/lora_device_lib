#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"

#include "lora_frame.h"

uint8_t Frame_encode(const void *key, const struct lora_frame *f, uint8_t *out, uint8_t max)
{
    return mock();
}

enum lora_frame_result Frame_decode(const void *appKey, const void *nwkSKey, const void *appSKey, void *in, uint8_t len, struct lora_frame *f)
{
    return mock();
}

uint16_t Frame_getPhyPayloadSize(uint8_t dataLen, uint8_t optsLen)
{
    return mock();
}

bool Frame_isUpstream(enum message_type type)
{
    return mock();
}
