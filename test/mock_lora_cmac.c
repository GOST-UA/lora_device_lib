#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include <string.h>

#include "cmocka.h"

#include "lora_cmac.h"

void LoraCMAC_init(struct lora_cmac_ctx *ctx, const struct lora_aes_ctx *aes_ctx)
{
}

void LoraCMAC_update(struct lora_cmac_ctx *ctx, const void *data, uint8_t len)
{
}

void LoraCMAC_finish(struct lora_cmac_ctx *ctx, void *out, size_t outMax)
{
    memset(out, 0, outMax);
}
