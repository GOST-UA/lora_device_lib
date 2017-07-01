 #include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"

#include "lora_aes.h"
#include "lora_cmac.h"

#include <string.h>

static void test_LoraCMAC_mlen0(void **user)
{
    struct lora_aes_ctx aes_ctx;
    struct lora_cmac_ctx cmac_ctx;
    static const uint8_t key[] = {0x2b,0x7e,0x15,0x16,0x28,0xae,0xd2,0xa6,0xab,0xf7,0x15,0x88,0x09,0xcf,0x4f,0x3c};
    static const uint8_t expectedOut[] = {0xbb,0x1d,0x69,0x29,0xe9,0x59,0x37,0x28,0x7f,0xa3,0x7d,0x12,0x9b,0x75,0x67,0x46};
    uint8_t out[16U];

    LoraAES_init(&aes_ctx, key);
    LoraCMAC_init(&cmac_ctx, &aes_ctx);
    LoraCMAC_finish(&cmac_ctx, out, sizeof(out));

    assert_memory_equal(expectedOut, &out, sizeof(expectedOut));    
}

static void test_LoraCMAC_mlen128(void **user)
{
    struct lora_aes_ctx aes_ctx;
    struct lora_cmac_ctx cmac_ctx;
    static const uint8_t key[] = {0x2b,0x7e,0x15,0x16,0x28,0xae,0xd2,0xa6,0xab,0xf7,0x15,0x88,0x09,0xcf,0x4f,0x3c};
    static const uint8_t m[] = {0x6b,0xc1,0xbe,0xe2,0x2e,0x40,0x9f,0x96,0xe9,0x3d,0x7e,0x11,0x73,0x93,0x17,0x2a};
    static const uint8_t expectedOut[] = {0x07,0x0a,0x16,0xb4,0x6b,0x4d,0x41,0x44,0xf7,0x9b,0xdd,0x9d,0xd0,0x4a,0x28,0x7c};
    uint8_t out[16U];

    LoraAES_init(&aes_ctx, key);
    LoraCMAC_init(&cmac_ctx, &aes_ctx);
    LoraCMAC_update(&cmac_ctx, m, sizeof(m));
    LoraCMAC_finish(&cmac_ctx, out, sizeof(out));

    assert_memory_equal(expectedOut, &out, sizeof(expectedOut));    
}

static void test_LoraCMAC_mlen320(void **user)
{
    struct lora_aes_ctx aes_ctx;
    struct lora_cmac_ctx cmac_ctx;
    static const uint8_t key[] = {0x2b,0x7e,0x15,0x16,0x28,0xae,0xd2,0xa6,0xab,0xf7,0x15,0x88,0x09,0xcf,0x4f,0x3c};
    static const uint8_t m[] = {0x6b,0xc1,0xbe,0xe2,0x2e,0x40,0x9f,0x96,0xe9,0x3d,0x7e,0x11,0x73,0x93,0x17,0x2a,0xae,0x2d,0x8a,0x57,0x1e,0x03,0xac,0x9c,0x9e,0xb7,0x6f,0xac,0x45,0xaf,0x8e,0x51,0x30,0xc8,0x1c,0x46,0xa3,0x5c,0xe4,0x11 };
    static const uint8_t expectedOut[] = {0xdf,0xa6,0x67,0x47,0xde,0x9a,0xe6,0x30,0x30,0xca,0x32,0x61,0x14,0x97,0xc8,0x27};
    uint8_t out[16U];

    LoraAES_init(&aes_ctx, key);
    LoraCMAC_init(&cmac_ctx, &aes_ctx);
    LoraCMAC_update(&cmac_ctx, m, sizeof(m));
    LoraCMAC_finish(&cmac_ctx, out, sizeof(out));

    assert_memory_equal(expectedOut, &out, sizeof(expectedOut));   
}

static void test_LoraCMAC_mlen512(void **user)
{
    struct lora_aes_ctx aes_ctx;
    struct lora_cmac_ctx cmac_ctx;
    static const uint8_t key[] = {0x2b,0x7e,0x15,0x16,0x28,0xae,0xd2,0xa6,0xab,0xf7,0x15,0x88,0x09,0xcf,0x4f,0x3c};
    static const uint8_t m[] = {0x6b,0xc1,0xbe,0xe2,0x2e,0x40,0x9f,0x96,0xe9,0x3d,0x7e,0x11,0x73,0x93,0x17,0x2a,0xae,0x2d,0x8a,0x57,0x1e,0x03,0xac,0x9c,0x9e,0xb7,0x6f,0xac,0x45,0xaf,0x8e,0x51,0x30,0xc8,0x1c,0x46,0xa3,0x5c,0xe4,0x11,0xe5,0xfb,0xc1,0x19,0x1a,0x0a,0x52,0xef,0xf6,0x9f,0x24,0x45,0xdf,0x4f,0x9b,0x17,0xad,0x2b,0x41,0x7b,0xe6,0x6c,0x37,0x10};
    static const uint8_t expectedOut[] = {0x51,0xf0,0xbe,0xbf,0x7e,0x3b,0x9d,0x92,0xfc,0x49,0x74,0x17,0x79,0x36,0x3c,0xfe};
    uint8_t out[16U];
    
    LoraAES_init(&aes_ctx, key);
    LoraCMAC_init(&cmac_ctx, &aes_ctx);
    LoraCMAC_update(&cmac_ctx, m, sizeof(m));
    LoraCMAC_finish(&cmac_ctx, out, sizeof(out));

    assert_memory_equal(expectedOut, &out, sizeof(expectedOut));   
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_LoraCMAC_mlen0),     
        cmocka_unit_test(test_LoraCMAC_mlen128),     
        cmocka_unit_test(test_LoraCMAC_mlen320),     
        cmocka_unit_test(test_LoraCMAC_mlen512)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
