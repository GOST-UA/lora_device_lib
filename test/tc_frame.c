#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"

#include "lora_frame.h"
#include "lora_aes.h"
#include "lora_cmac.h"

#include <string.h>

void LoraAES_init(struct lora_aes_ctx *ctx, const uint8_t *key)
{
}

void LoraAES_encrypt(const struct lora_aes_ctx *ctx, uint8_t *s)
{
}

void LoraAES_decrypt(const struct lora_aes_ctx *ctx, uint8_t *s)
{
}

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

static void test_LoraFrame_data_encode_empty(void **user)
{
    const uint8_t expected[] = "\x02\x00\x02\x00\x00\x00\x00\x01\x00\x00\x00\x00";
    const uint8_t dummyKey[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
    struct lora_frame input;
    (void)memset(&input, 0, sizeof(input));
    uint8_t outLen;
    uint8_t out[UINT8_MAX];

    input.type = FRAME_TYPE_DATA_UNCONFIRMED_UP;
    input.fields.data.counter = 256;
    input.fields.data.devAddr = 512;

    outLen = LoraFrame_encode(dummyKey, &input, out, sizeof(out));
    
    assert_int_equal(sizeof(expected)-1U, outLen);
    assert_memory_equal(expected, out, sizeof(expected)-1U);
}

static void test_LoraFrame_data_decode_empty(void **user)
{
    uint8_t input[] = "\x02\x00\x02\x00\x00\x00\x00\x01\x00\x00\x00\x00";
    const uint8_t dummyKey[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
    struct lora_frame expected;
    struct lora_frame output;
    enum lora_frame_result expectedResult = LORA_FRAME_OK;
    enum lora_frame_result result;

    (void)memset(&expected, 0, sizeof(expected));
    expected.type = FRAME_TYPE_DATA_UNCONFIRMED_UP;
    expected.fields.data.counter = 256;
    expected.fields.data.devAddr = 512;

    result = LoraFrame_decode(dummyKey, dummyKey, dummyKey, input, sizeof(input)-1U, &output);

    assert_int_equal(expectedResult, result);
    assert_memory_equal(&expected, &output, sizeof(expected));
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_LoraFrame_data_encode_empty),        
        cmocka_unit_test(test_LoraFrame_data_decode_empty),        
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}

