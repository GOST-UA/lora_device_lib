#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"

#include "lora_frame.h"
#include "lora_aes.h"
#include "lora_cmac.h"

#include <string.h>

static void test_LoraFrame_data_encode_empty(void **user)
{
    const uint8_t devAddr[] = {0x00, 0x00, 0x00, 0x00};
    const uint8_t expected[] = "\x02\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00";
    const uint8_t dummyKey[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
    struct lora_frame input;
    (void)memset(&input, 0, sizeof(input));
    uint8_t outLen;
    uint8_t out[UINT8_MAX];

    input.type = FRAME_TYPE_DATA_UNCONFIRMED_UP;
    input.fields.data.counter = 256;
    (void)memcpy(&input.fields.data.devAddr, devAddr, sizeof(input.fields.data.devAddr));
     
    outLen = Frame_encode(dummyKey, &input, out, sizeof(out));
    
    assert_int_equal(sizeof(expected)-1U, outLen);
    assert_memory_equal(expected, out, sizeof(expected)-1U);
}

static void test_LoraFrame_data_decode_empty(void **user)
{
    const uint8_t devAddr[] = {0x00, 0x00, 0x00, 0x00};
    uint8_t input[] = "\x02\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00";
    const uint8_t dummyKey[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
    struct lora_frame expected;
    struct lora_frame output;
    enum lora_frame_result expectedResult = LORA_FRAME_OK;
    enum lora_frame_result result;

    (void)memset(&expected, 0, sizeof(expected));
    expected.type = FRAME_TYPE_DATA_UNCONFIRMED_UP;
    expected.fields.data.counter = 256;
    (void)memcpy(&expected.fields.data.devAddr, devAddr, sizeof(expected.fields.data.devAddr));

    result = Frame_decode(dummyKey, dummyKey, dummyKey, input, sizeof(input)-1U, &output);

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


