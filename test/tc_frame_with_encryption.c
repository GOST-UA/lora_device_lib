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
                                
    struct lora_frame_data f;
    enum lora_frame_type type;
    
    (void)memset(&f, 0, sizeof(f));
    
    type = FRAME_TYPE_DATA_UNCONFIRMED_UP;
    f.data = payload;
    f.dataLen = sizeof(payload)-1;
    
    
    retval = Frame_putData(type, key, &f, buffer, sizeof(buffer));
    
    assert_int_equal(sizeof(expected)-1U, retval);
    
    assert_memory_equal(expected, buffer, retval);    
}

static void test_Frame_encode_joinReq(void **user)
{
    uint8_t retval;
    uint8_t buffer[UINT8_MAX];
    const uint8_t key[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
    const uint8_t expected[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x71\x84\x9D\xAA";
    
    struct lora_frame_join_request f;
    
    (void)memset(&f, 0, sizeof(f));
    
    retval = Frame_putJoinRequest(key, &f, buffer, sizeof(buffer));
    
    assert_int_equal(sizeof(expected)-1U, retval);
    
    assert_memory_equal(expected, buffer, retval);    
}

static void test_Frame_encode_joinAccept(void **user)
{
    uint8_t retval;
    uint8_t buffer[UINT8_MAX];
    const uint8_t key[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
    const uint8_t expected[] = "\x20\xE3\xDE\x10\x87\x95\xF7\x76\xB8\x03\x76\x10\xEF\x78\x69\xB5\xB3";
    
    struct lora_frame_join_accept f;
    
    (void)memset(&f, 0, sizeof(f));
    
    retval = Frame_putJoinAccept(key, &f, buffer, sizeof(buffer));
    
    assert_int_equal(sizeof(expected)-1U, retval);
    
    assert_memory_equal(expected, buffer, retval);    
}

static void test_Frame_encode_joinAccept_withCFList(void **user)
{
    uint8_t retval;
    uint8_t buffer[UINT8_MAX];
    const uint8_t key[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
    const uint8_t expected[] = "\x20\x14\x0F\x0F\x10\x11\xB5\x22\x3D\x79\x58\x77\x17\xFF\xD9\xEC\x3A\xB6\x05\xA8\x02\xAC\x97\xDD\xE7\xAC\xF0\x5C\x87\xEF\xAC\x47\xAF";
    
    struct lora_frame_join_accept f;
    
    (void)memset(&f, 0, sizeof(f));
    
    f.cfListLen = sizeof(f.cfList);
    
    retval = Frame_putJoinAccept(key, &f, buffer, sizeof(buffer));
    
    assert_int_equal(sizeof(expected)-1U, retval);
    
    assert_memory_equal(expected, buffer, retval);    
}

static void test_Frame_encode_croftExample(void **user)
{
    uint8_t retval;
    uint8_t buffer[UINT8_MAX];
    const uint8_t payload[] = "{\"name\":\"Turiphro\",\"count\":13,\"water\":true}";
    const uint8_t key[] = {0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C};
    const uint8_t expected[] = {0x80, 0x8F, 0x77, 0xBB, 0x07, 0x00, 0x02, 0x00, 0x06, 0xBD, 0x33, 0x42, 0xA1, 0x9F, 0xCC, 0x3C, 0x8D, 0x6B, 0xCB, 0x5F, 0xDB, 0x05, 0x48, 0xDB, 0x4D, 0xC8, 0x50, 0x14, 0xAE, 0xEB, 0xFE, 0x0B, 0x54, 0xB1, 0xC9, 0x98, 0xDE, 0xF5, 0x3E, 0x97, 0x9B, 0x70, 0x1D, 0xAB, 0xB0, 0x45, 0x30, 0x0E, 0xF8, 0x69, 0x9C, 0x38, 0xFC, 0x1A, 0x34, 0xD5};
    
    struct lora_frame_data f;
    enum lora_frame_type type;
    
    (void)memset(&f, 0, sizeof(f));
    
    type = FRAME_TYPE_DATA_CONFIRMED_UP;
    f.devAddr = 0x07BB778F;
    f.counter = 2;
    f.port = 6;
    f.data = payload;
    f.dataLen = sizeof(payload)-1;
    
    retval = Frame_putData(type, key, &f, buffer, sizeof(buffer));
    
    assert_int_equal(sizeof(expected), retval);
    
    assert_memory_equal(expected, buffer, retval);    
}

static void test_Frame_decode_unconfirmedUp(void **user)
{
    const uint8_t data[] = "hello world";
    const uint8_t key[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";    
    uint8_t input[] =  "\x40\x00\x00\x00\x00\x00\x00\x00\x00\xBD\x1D\x9E\x61\x6F\xB5\xFB\x03\x22\x02\x52\xAB\xDC\x77\x2F";
                                
    struct lora_frame f;
    
    bool result = Frame_decode(key, key, key, input, sizeof(input)-1U, &f);
    
    assert_true(result);    
    
    assert_int_equal(FRAME_TYPE_DATA_UNCONFIRMED_UP, f.type);    
    
    assert_int_equal(0, f.fields.data.optsLen);    
    
    assert_int_equal(sizeof(data)-1, f.fields.data.dataLen);    
    assert_memory_equal(data, f.fields.data.data, f.fields.data.dataLen);    
}

static void test_Frame_decode_unconfirmedUp_withOpts(void **user)
{
    const uint8_t key[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";    
    uint8_t input[] = "\x40\x00\x00\x00\x00\x05\x00\x00\x68\x65\x6C\x6C\x6F\x00\xBD\x1D\x9E\x61\x6F\xB5\xFB\x03\x22\x02\x52\x93\xCA\x7E\x69";
    
    struct lora_frame f;
    
    bool result = Frame_decode(key, key, key, input, sizeof(input)-1U, &f);
    
    assert_false(result);        
}

static void test_Frame_decode_joinReq(void **user)
{
    const uint8_t key[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
    uint8_t input[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x71\x84\x9D\xAA";
    
    struct lora_frame f;
    
    bool result = Frame_decode(key, key, key, input, sizeof(input)-1U, &f);
    
    assert_true(result);    
    
    assert_int_equal(FRAME_TYPE_JOIN_REQ, f.type);    
}

static void test_Frame_decode_joinAccept(void **user)
{
    const uint8_t key[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
    uint8_t input[] = "\x20\xE3\xDE\x10\x87\x95\xF7\x76\xB8\x03\x76\x10\xEF\x78\x69\xB5\xB3";
    
    struct lora_frame f;
    
    bool result = Frame_decode(key, key, key, input, sizeof(input)-1U, &f);
    
    assert_true(result);    
    
    assert_int_equal(FRAME_TYPE_JOIN_ACCEPT, f.type);    
}

static void test_Frame_decode_joinAccept_withCFList(void **user)
{    
    const uint8_t key[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
    uint8_t input[] = "\x20\x14\x0F\x0F\x10\x11\xB5\x22\x3D\x79\x58\x77\x17\xFF\xD9\xEC\x3A\xB6\x05\xA8\x02\xAC\x97\xDD\xE7\xAC\xF0\x5C\x87\xEF\xAC\x47\xAF";
    
    struct lora_frame f;
    
    bool result = Frame_decode(key, key, key, input, sizeof(input)-1U, &f);
    
    assert_true(result);    
    
    assert_int_equal(FRAME_TYPE_JOIN_ACCEPT, f.type);    
}

static void test_Frame_decode_croftExample(void **user)
{
    const uint8_t key[] = {0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C};
    uint8_t input[] = {0x80, 0x8F, 0x77, 0xBB, 0x07, 0x00, 0x02, 0x00, 0x06, 0xBD, 0x33, 0x42, 0xA1, 0x9F, 0xCC, 0x3C, 0x8D, 0x6B, 0xCB, 0x5F, 0xDB, 0x05, 0x48, 0xDB, 0x4D, 0xC8, 0x50, 0x14, 0xAE, 0xEB, 0xFE, 0x0B, 0x54, 0xB1, 0xC9, 0x98, 0xDE, 0xF5, 0x3E, 0x97, 0x9B, 0x70, 0x1D, 0xAB, 0xB0, 0x45, 0x30, 0x0E, 0xF8, 0x69, 0x9C, 0x38, 0xFC, 0x1A, 0x34, 0xD5};
    
    struct lora_frame f;
    
    bool result = Frame_decode(key, key, key, input, sizeof(input), &f);
    
    assert_true(result);    
    
    assert_int_equal(FRAME_TYPE_DATA_CONFIRMED_UP, f.type);    
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_Frame_encode_unconfirmedUp),        
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
