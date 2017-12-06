#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"

#include "lora_frame.h"

size_t Frame_putData(enum lora_frame_type type, const void *key, const struct lora_frame_data *f, void *out, size_t max)
{
    return mock();
}

size_t Frame_putJoinRequest(const void *key, const struct lora_frame_join_request *f, void *out, size_t max)
{
    return mock();
}

size_t Frame_putJoinAccept(const void *key, const struct lora_frame_join_accept *f, void *out, size_t max)
{
    return mock();
}

bool Frame_decode(const void *appKey, const void *nwkSKey, const void *appSKey, void *in, size_t len, struct lora_frame *f)
{
    return mock();
}

size_t Frame_getPhyPayloadSize(size_t dataLen, size_t optsLen)
{
    return mock();
}

bool Frame_isUpstream(enum lora_frame_type type)
{
    return mock();
}
