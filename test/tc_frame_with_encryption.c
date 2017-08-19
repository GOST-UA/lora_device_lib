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
    const uint8_t expected[] =  "\x02\x00\x00\x00\x00\x00\x00\x00\x00\xBD\x1D\x9E\x61\x6F\xB5\xFB\x03\x22\x02\x52\x3F\xEA\x6C\xB2";
                                
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
    
    const uint8_t expected[] = "\x02\x00\x00\x00\x00\x05\x00\x00\x68\x65\x6C\x6C\x6F\x00\xBD\x1D\x9E\x61\x6F\xB5\xFB\x03\x22\x02\x52\xD9\xB7\xC6\xE6";
    
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
    const uint8_t expected[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xAA\x9D\x84\x71";
    
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
    const uint8_t expected[] = "\x01\xD4\x87\x3C\x68\xF7\x44\x73\x8A\xF3\x47\x19\xEF\x2F\xA4\xC2\x94";
    
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
    const uint8_t expected[] = "\x01\x14\x0F\x0F\x10\x11\xB5\x22\x3D\x79\x58\x77\x17\xFF\xD9\xEC\x3A\xE0\xBA\x0C\xDE\x4C\xAF\xE4\xEE\xD3\x6E\xF2\x8C\xBA\x39\xE7\x9C";
    
    struct lora_frame f;
    
    (void)memset(&f, 0, sizeof(f));
    
    f.type = FRAME_TYPE_JOIN_ACCEPT;
    f.fields.joinAccept.cfListLen = sizeof(f.fields.joinAccept.cfList);
    
    retval = Frame_encode(key, &f, buffer, sizeof(buffer));
    
    assert_int_equal(sizeof(expected)-1U, retval);
    
    assert_memory_equal(expected, buffer, retval);    
}

static void test_Frame_decode_unconfirmedUp(void **user)
{
    const uint8_t data[] = "hello world";
    const uint8_t key[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";    
    uint8_t input[] =  "\x02\x00\x00\x00\x00\x00\x00\x00\x00\xBD\x1D\x9E\x61\x6F\xB5\xFB\x03\x22\x02\x52\x3F\xEA\x6C\xB2";
                                
    struct lora_frame f;
    
    enum lora_frame_result result = Frame_decode(key, key, key, input, sizeof(input)-1U, &f);
    
    assert_int_equal(LORA_FRAME_OK, result);    
    
    assert_int_equal(FRAME_TYPE_DATA_CONFIRMED_UP, f.type);    
    
    assert_int_equal(0, f.fields.data.optsLen);    
    
    assert_int_equal(sizeof(data)-1, f.fields.data.dataLen);    
    assert_memory_equal(data, f.fields.data.data, f.fields.data.dataLen);    
}

static void test_Frame_decode_unconfirmedUp_withOpts(void **user)
{
    const uint8_t data[] = "hello world";
    const uint8_t opts[] = "hello";
    const uint8_t key[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
    
    uint8_t input[] = "\x02\x00\x00\x00\x00\x05\x00\x00\x68\x65\x6C\x6C\x6F\x00\xBD\x1D\x9E\x61\x6F\xB5\xFB\x03\x22\x02\x52\xD9\xB7\xC6\xE6";
    
    struct lora_frame f;
    
    enum lora_frame_result result = Frame_decode(key, key, key, input, sizeof(input)-1U, &f);
    
    assert_int_equal(LORA_FRAME_OK, result);    
    
    assert_int_equal(FRAME_TYPE_DATA_CONFIRMED_UP, f.type);    
    
    assert_int_equal(sizeof(opts)-1, f.fields.data.optsLen);    
    assert_memory_equal(opts, f.fields.data.opts, f.fields.data.optsLen);    
    
    assert_int_equal(sizeof(data)-1, f.fields.data.dataLen);    
    assert_memory_equal(data, f.fields.data.data, f.fields.data.dataLen);    
}

static void test_Frame_decode_joinReq(void **user)
{
    const uint8_t key[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
    uint8_t input[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xAA\x9D\x84\x71";
    
    struct lora_frame f;
    
    enum lora_frame_result result = Frame_decode(key, key, key, input, sizeof(input)-1U, &f);
    
    assert_int_equal(LORA_FRAME_OK, result);    
    
    assert_int_equal(FRAME_TYPE_JOIN_REQ, f.type);    
}

static void test_Frame_decode_joinAccept(void **user)
{
    const uint8_t key[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
    uint8_t input[] = "\x01\xD4\x87\x3C\x68\xF7\x44\x73\x8A\xF3\x47\x19\xEF\x2F\xA4\xC2\x94";
    
    struct lora_frame f;
    
    enum lora_frame_result result = Frame_decode(key, key, key, input, sizeof(input)-1U, &f);
    
    assert_int_equal(LORA_FRAME_OK, result);    
    
    assert_int_equal(FRAME_TYPE_JOIN_ACCEPT, f.type);    
}

static void test_Frame_decode_joinAccept_withCFList(void **user)
{    
    const uint8_t key[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
    uint8_t input[] = "\x01\x14\x0F\x0F\x10\x11\xB5\x22\x3D\x79\x58\x77\x17\xFF\xD9\xEC\x3A\xE0\xBA\x0C\xDE\x4C\xAF\xE4\xEE\xD3\x6E\xF2\x8C\xBA\x39\xE7\x9C";
    
    struct lora_frame f;
    
    enum lora_frame_result result = Frame_decode(key, key, key, input, sizeof(input)-1U, &f);
    
    assert_int_equal(LORA_FRAME_OK, result);    
    
    assert_int_equal(FRAME_TYPE_JOIN_ACCEPT, f.type);    
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_Frame_encode_unconfirmedUp),        
        cmocka_unit_test(test_Frame_encode_unconfirmedUp_withOpts),        
        cmocka_unit_test(test_Frame_encode_joinReq),        
        cmocka_unit_test(test_Frame_encode_joinAccept),        
        cmocka_unit_test(test_Frame_encode_joinAccept_withCFList),        
        
        cmocka_unit_test(test_Frame_decode_unconfirmedUp),        
        cmocka_unit_test(test_Frame_decode_unconfirmedUp_withOpts),        
        cmocka_unit_test(test_Frame_decode_joinReq),        
        cmocka_unit_test(test_Frame_decode_joinAccept),        
        cmocka_unit_test(test_Frame_decode_joinAccept_withCFList),                
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
