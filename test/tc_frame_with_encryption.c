#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"

#include "lora_frame.h"
#include "lora_aes.h"
#include "lora_cmac.h"

#include <string.h>

static void test_Frame_encode_unconfirmedUp(void **user)
{
    uint8_t retval;
    uint8_t buffer[UINT8_MAX];
    const uint8_t payload[] = "hello world";
    const uint8_t key[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
    
    const uint8_t expected[] = "\x02\x00\x00\x00\x00\x00\x00\x00\x00hello world\x00\x00\x00\x00";
    
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
    
    const uint8_t expected[] = "\x02\x00\x00\x00\x00\x05\x00\x00hello\x00hello world\x00\x00\x00\x00";
    
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
    const uint8_t expected[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
    
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
    const uint8_t expected[] = "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
    
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
    const uint8_t expected[] = "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
    
    struct lora_frame f;
    
    (void)memset(&f, 0, sizeof(f));
    
    f.type = FRAME_TYPE_JOIN_ACCEPT;
    f.fields.joinAccept.cfListLen = sizeof(f.fields.joinAccept.cfList);
    
    retval = Frame_encode(key, &f, buffer, sizeof(buffer));
    
    assert_int_equal(sizeof(expected)-1U, retval);
    
    assert_memory_equal(expected, buffer, retval);    
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_Frame_encode_unconfirmedUp),        
        cmocka_unit_test(test_Frame_encode_unconfirmedUp_withOpts),        
        cmocka_unit_test(test_Frame_encode_joinReq),        
        cmocka_unit_test(test_Frame_encode_joinAccept),        
        cmocka_unit_test(test_Frame_encode_joinAccept_withCFList),        
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
