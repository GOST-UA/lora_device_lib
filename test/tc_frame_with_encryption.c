#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"

#include "lora_frame.h"
#include "lora_aes.h"
#include "lora_cmac.h"

#include <string.h>

/* note these tests only exist to catch when something changes, they don't validate the correct
 * implementation of the lorawan encryption since lorawan don't provide test vectors */

static void test_Frame_encode_unconfirmedUp(void **user)
{
    uint8_t retval;
    uint8_t buffer[UINT8_MAX];
    const uint8_t payload[] = "hello world";
    const uint8_t key[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";    
    const uint8_t expected[] =  "\x40\x00\x00\x00\x00\x00\x00\x00\x00\xBD\x1D\x9E\x61\x6F\xB5\xFB\x03\x22\x02\x52\xAB\xDC\x77\x2F";
                                
    struct lora_frame f;
    
    (void)memset(&f, 0, sizeof(f));
    
    f.type = FRAME_TYPE_DATA_UNCONFIRMED_UP;
    f.fields.data.data = payload;
    f.fields.data.dataLen = sizeof(payload)-1;
    
    retval = Frame_encode(key, &f, buffer, sizeof(buffer));
    
    assert_int_equal(sizeof(expected)-1U, retval);
    
    assert_memory_equal(expected, buffer, retval);    
}

static void test_Frame_encode_unconfirmedUp_withOpts(void **user)
{
    uint8_t retval;
    uint8_t buffer[UINT8_MAX];
    const uint8_t data[] = "hello world";
    const uint8_t opts[] = "hello";
    const uint8_t key[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
    
    const uint8_t expected[] = "\x40\x00\x00\x00\x00\x05\x00\x00\x68\x65\x6C\x6C\x6F\x00\xBD\x1D\x9E\x61\x6F\xB5\xFB\x03\x22\x02\x52\x93\xCA\x7E\x69";
    
    struct lora_frame f;
    
    (void)memset(&f, 0, sizeof(f));
    
    f.type = FRAME_TYPE_DATA_UNCONFIRMED_UP;
    f.fields.data.data = data;
    f.fields.data.dataLen = sizeof(data)-1U;
    f.fields.data.opts = opts;
    f.fields.data.optsLen = sizeof(opts)-1U;
    
    retval = Frame_encode(key, &f, buffer, sizeof(buffer));
    
    assert_int_equal(sizeof(expected)-1U, retval);
    
    assert_memory_equal(expected, buffer, retval);    
}

static void test_Frame_encode_joinReq(void **user)
{
    uint8_t retval;
    uint8_t buffer[UINT8_MAX];
    const uint8_t key[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
    const uint8_t expected[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x71\x84\x9D\xAA";
    
    struct lora_frame f;
    
    (void)memset(&f, 0, sizeof(f));
    
    f.type = FRAME_TYPE_JOIN_REQ;
    
    retval = Frame_encode(key, &f, buffer, sizeof(buffer));
    
    assert_int_equal(sizeof(expected)-1U, retval);
    
    assert_memory_equal(expected, buffer, retval);    
}

static void test_Frame_encode_joinAccept(void **user)
{
    uint8_t retval;
    uint8_t buffer[UINT8_MAX];
    const uint8_t key[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
    const uint8_t expected[] = "\x20\xE3\xDE\x10\x87\x95\xF7\x76\xB8\x03\x76\x10\xEF\x78\x69\xB5\xB3";
    
    struct lora_frame f;
    
    (void)memset(&f, 0, sizeof(f));
    
    f.type = FRAME_TYPE_JOIN_ACCEPT;
    
    retval = Frame_encode(key, &f, buffer, sizeof(buffer));
    
    assert_int_equal(sizeof(expected)-1U, retval);
    
    assert_memory_equal(expected, buffer, retval);    
}

static void test_Frame_encode_joinAccept_withCFList(void **user)
{
    uint8_t retval;
    uint8_t buffer[UINT8_MAX];
    const uint8_t key[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
    const uint8_t expected[] = "\x20\x14\x0F\x0F\x10\x11\xB5\x22\x3D\x79\x58\x77\x17\xFF\xD9\xEC\x3A\xB6\x05\xA8\x02\xAC\x97\xDD\xE7\xAC\xF0\x5C\x87\xEF\xAC\x47\xAF";
    
    struct lora_frame f;
    
    (void)memset(&f, 0, sizeof(f));
    
    f.type = FRAME_TYPE_JOIN_ACCEPT;
    f.fields.joinAccept.cfListLen = sizeof(f.fields.joinAccept.cfList);
    
    retval = Frame_encode(key, &f, buffer, sizeof(buffer));
    
    assert_int_equal(sizeof(expected)-1U, retval);
    
    assert_memory_equal(expected, buffer, retval);    
}

static void test_Frame_encode_croftExample(void **user)
{
    uint8_t retval;
    uint8_t buffer[UINT8_MAX];
    const uint8_t payload[] = "{\"name\":\"Turiphro\",\"count\":13,\"water\":true}";
    const uint8_t key[] = {0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C};
    const uint8_t expected[] = "\x80\x8f\x77\xbb\x07\x00\x02\x00\x06\xbd\x33\x42\xa1\x9f\xcc\x3c\x8d\x6b\xcb\x5f\xdb\x05\x48\xdb\x4d\xc8\x50\x14\xae\xeb\xfe\xb\x54\xb1\xc9\x98\xde\xf5\x3e\x97\x9b\x70\x1d\xab\xb0\x45\x30\x0e\xf8\x69\x9c\x38\xfc\x1a\x34\xd5";
    
    struct lora_frame f;
    
    (void)memset(&f, 0, sizeof(f));
    
    f.type = FRAME_TYPE_DATA_CONFIRMED_UP;
    f.fields.data.devAddr = 0x07BB778F;
    f.fields.data.counter = 2;
    f.fields.data.port = 6;
    f.fields.data.data = payload;
    f.fields.data.dataLen = sizeof(payload)-1;
    
    retval = Frame_encode(key, &f, buffer, sizeof(buffer));
    
    assert_int_equal(sizeof(expected)-1U, retval);
    
    assert_memory_equal(expected, buffer, retval);    
}

static void test_Frame_decode_unconfirmedUp(void **user)
{
    const uint8_t data[] = "hello world";
    const uint8_t key[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";    
    uint8_t input[] =  "\x40\x00\x00\x00\x00\x00\x00\x00\x00\xBD\x1D\x9E\x61\x6F\xB5\xFB\x03\x22\x02\x52\xAB\xDC\x77\x2F";
                                
    struct lora_frame f;
    
    enum lora_frame_result result = Frame_decode(key, key, key, input, sizeof(input)-1U, &f);
    
    assert_int_equal(LORA_FRAME_OK, result);    
    
    assert_int_equal(FRAME_TYPE_DATA_UNCONFIRMED_UP, f.type);    
    
    assert_int_equal(0, f.fields.data.optsLen);    
    
    assert_int_equal(sizeof(data)-1, f.fields.data.dataLen);    
    assert_memory_equal(data, f.fields.data.data, f.fields.data.dataLen);    
}

static void test_Frame_decode_unconfirmedUp_withOpts(void **user)
{
    const uint8_t data[] = "hello world";
    const uint8_t opts[] = "hello";
    const uint8_t key[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
    
    uint8_t input[] = "\x40\x00\x00\x00\x00\x05\x00\x00\x68\x65\x6C\x6C\x6F\x00\xBD\x1D\x9E\x61\x6F\xB5\xFB\x03\x22\x02\x52\x93\xCA\x7E\x69";
    
    struct lora_frame f;
    
    enum lora_frame_result result = Frame_decode(key, key, key, input, sizeof(input)-1U, &f);
    
    assert_int_equal(LORA_FRAME_OK, result);    
    
    assert_int_equal(FRAME_TYPE_DATA_UNCONFIRMED_UP, f.type);    
    
    assert_int_equal(sizeof(opts)-1, f.fields.data.optsLen);    
    assert_memory_equal(opts, f.fields.data.opts, f.fields.data.optsLen);    
    
    assert_int_equal(sizeof(data)-1, f.fields.data.dataLen);    
    assert_memory_equal(data, f.fields.data.data, f.fields.data.dataLen);    
}

static void test_Frame_decode_joinReq(void **user)
{
    const uint8_t key[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
    uint8_t input[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x71\x84\x9D\xAA";
    
    struct lora_frame f;
    
    enum lora_frame_result result = Frame_decode(key, key, key, input, sizeof(input)-1U, &f);
    
    assert_int_equal(LORA_FRAME_OK, result);    
    
    assert_int_equal(FRAME_TYPE_JOIN_REQ, f.type);    
}

static void test_Frame_decode_joinAccept(void **user)
{
    const uint8_t key[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
    uint8_t input[] = "\x20\xE3\xDE\x10\x87\x95\xF7\x76\xB8\x03\x76\x10\xEF\x78\x69\xB5\xB3";
    
    struct lora_frame f;
    
    enum lora_frame_result result = Frame_decode(key, key, key, input, sizeof(input)-1U, &f);
    
    assert_int_equal(LORA_FRAME_OK, result);    
    
    assert_int_equal(FRAME_TYPE_JOIN_ACCEPT, f.type);    
}

static void test_Frame_decode_joinAccept_withCFList(void **user)
{    
    const uint8_t key[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
    uint8_t input[] = "\x20\x14\x0F\x0F\x10\x11\xB5\x22\x3D\x79\x58\x77\x17\xFF\xD9\xEC\x3A\xB6\x05\xA8\x02\xAC\x97\xDD\xE7\xAC\xF0\x5C\x87\xEF\xAC\x47\xAF";
    
    struct lora_frame f;
    
    enum lora_frame_result result = Frame_decode(key, key, key, input, sizeof(input)-1U, &f);
    
    assert_int_equal(LORA_FRAME_OK, result);    
    
    assert_int_equal(FRAME_TYPE_JOIN_ACCEPT, f.type);    
}

static void test_Frame_decode_croftExample(void **user)
{
    const uint8_t key[] = {0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C};
    uint8_t input[] = "\x80\x8f\x77\xbb\x07\x00\x02\x00\x06\xbd\x33\x42\xa1\x9f\xcc\x3c\x8d\x6b\xcb\x5f\xdb\x05\x48\xdb\x4d\xc8\x50\x14\xae\xeb\xfe\xb\x54\xb1\xc9\x98\xde\xf5\x3e\x97\x9b\x70\x1d\xab\xb0\x45\x30\x0e\xf8\x69\x9c\x38\xfc\x1a\x34\xd5";
    
    struct lora_frame f;
    
    enum lora_frame_result result = Frame_decode(key, key, key, input, sizeof(input)-1U, &f);
    
    assert_int_equal(LORA_FRAME_OK, result);    
    
    assert_int_equal(FRAME_TYPE_DATA_CONFIRMED_UP, f.type);    
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_Frame_encode_unconfirmedUp),        
        cmocka_unit_test(test_Frame_encode_unconfirmedUp_withOpts),        
        cmocka_unit_test(test_Frame_encode_joinReq),        
        cmocka_unit_test(test_Frame_encode_joinAccept),        
        cmocka_unit_test(test_Frame_encode_joinAccept_withCFList),        
        cmocka_unit_test(test_Frame_encode_croftExample),        
        
        cmocka_unit_test(test_Frame_decode_unconfirmedUp),        
        cmocka_unit_test(test_Frame_decode_unconfirmedUp_withOpts),        
        cmocka_unit_test(test_Frame_decode_joinReq),        
        cmocka_unit_test(test_Frame_decode_joinAccept),        
        cmocka_unit_test(test_Frame_decode_joinAccept_withCFList),                
        cmocka_unit_test(test_Frame_decode_croftExample),                
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
