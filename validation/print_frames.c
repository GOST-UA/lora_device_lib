/* Copyright (c) 2017 Cameron Harper
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * */

/* At time of reading the LoraWAN specification (102?) doesn't provide any 
 * "smoke test" frame examples in the appendices. This is a nuisance
 * because there is no simple way to make sure you have implemented
 * the frame codec and cryptography correctly.
 * 
 * I've had to pick through the reference implementation to verify 
 * my implementation. 
 * 
 * This file prints out a couple of frame examples for inspection and
 * comparison.
 * 
 * */


#include "lora_frame.h"
#include "lora_aes.h"
#include "lora_cmac.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static void print_hex(const char *name, const uint8_t *data, size_t len)
{
    size_t i;
    int offset;
    
    printf("%s: %n", name, &offset);
    
    for(i=0; i < (50-offset); i++){
        
        printf(" ");
    }
    
    printf("\"");
    
    for(i=0; i < len; i++){
        
        printf("\\x%02X", data[i]);
    }
    
    printf("\"\n");
}

static void Frame_encode_unconfirmedUp(void)
{
    uint8_t retval;
    uint8_t buffer[UINT8_MAX];
    const uint8_t payload[] = "hello world";
    const uint8_t key[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
    
    struct lora_frame f;
    
    (void)memset(&f, 0, sizeof(f));
    
    f.type = FRAME_TYPE_DATA_UNCONFIRMED_UP;
    f.fields.data.data = payload;
    f.fields.data.dataLen = sizeof(payload)-1;
    
    retval = Frame_encode(key, &f, buffer, sizeof(buffer));
    
    print_hex(__FUNCTION__, buffer, retval);
}

static void Frame_encode_unconfirmedUp_upCount(void)
{
    uint8_t retval;
    uint8_t buffer[UINT8_MAX];
    const uint8_t payload[] = "hello world";
    const uint8_t key[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
    
    struct lora_frame f;
    
    (void)memset(&f, 0, sizeof(f));
    
    f.type = FRAME_TYPE_DATA_UNCONFIRMED_UP;
    f.fields.data.data = payload;
    f.fields.data.dataLen = sizeof(payload)-1;
    f.fields.data.counter = 256;
    
    retval = Frame_encode(key, &f, buffer, sizeof(buffer));
    
    print_hex(__FUNCTION__, buffer, retval);
}

static void Frame_encode_unconfirmedUp_withOpts(void)
{
    uint8_t retval;
    uint8_t buffer[UINT8_MAX];
    const uint8_t data[] = "hello world";
    const uint8_t opts[] = "hello";
    const uint8_t key[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
    
    struct lora_frame f;
    
    (void)memset(&f, 0, sizeof(f));
    
    f.type = FRAME_TYPE_DATA_UNCONFIRMED_UP;
    f.fields.data.data = data;
    f.fields.data.dataLen = sizeof(data)-1U;
    f.fields.data.opts = opts;
    f.fields.data.optsLen = sizeof(opts)-1U;
    
    retval = Frame_encode(key, &f, buffer, sizeof(buffer));
    
    print_hex(__FUNCTION__, buffer, retval);
}

static void Frame_encode_unconfirmedUp_withOpts_upCount(void)
{
    uint8_t retval;
    uint8_t buffer[UINT8_MAX];
    const uint8_t data[] = "hello world";
    const uint8_t opts[] = "hello";
    const uint8_t key[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
    
    struct lora_frame f;
    
    (void)memset(&f, 0, sizeof(f));
    
    f.type = FRAME_TYPE_DATA_UNCONFIRMED_UP;
    f.fields.data.data = data;
    f.fields.data.dataLen = sizeof(data)-1U;
    f.fields.data.opts = opts;
    f.fields.data.optsLen = sizeof(opts)-1U;
    f.fields.data.counter = 1U;
    
    retval = Frame_encode(key, &f, buffer, sizeof(buffer));
    
    print_hex(__FUNCTION__, buffer, retval);
}

static void Frame_encode_joinReq(void)
{
    uint8_t retval;
    uint8_t buffer[UINT8_MAX];
    const uint8_t key[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
    
    struct lora_frame f;
    
    (void)memset(&f, 0, sizeof(f));
    
    f.type = FRAME_TYPE_JOIN_REQ;
    
    retval = Frame_encode(key, &f, buffer, sizeof(buffer));
    
    print_hex(__FUNCTION__, buffer, retval);
}

static void Frame_encode_joinAccept(void)
{
    uint8_t retval;
    uint8_t buffer[UINT8_MAX];
    const uint8_t key[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
    
    struct lora_frame f;
    
    (void)memset(&f, 0, sizeof(f));
    
    f.type = FRAME_TYPE_JOIN_ACCEPT;
    
    retval = Frame_encode(key, &f, buffer, sizeof(buffer));
    
    print_hex(__FUNCTION__, buffer, retval);
}

static void Frame_encode_joinAccept_withCFList(void)
{
    uint8_t retval;
    uint8_t buffer[UINT8_MAX];
    const uint8_t key[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
    
    struct lora_frame f;
    
    (void)memset(&f, 0, sizeof(f));
    
    f.type = FRAME_TYPE_JOIN_ACCEPT;
    f.fields.joinAccept.cfListLen = sizeof(f.fields.joinAccept.cfList);
    
    retval = Frame_encode(key, &f, buffer, sizeof(buffer));
    
    print_hex(__FUNCTION__, buffer, retval);
}

int main(int argc, const char **argv)
{
    Frame_encode_joinAccept();
    Frame_encode_joinAccept_withCFList();
    Frame_encode_joinReq();
    Frame_encode_unconfirmedUp();
    Frame_encode_unconfirmedUp_upCount();
    Frame_encode_unconfirmedUp_withOpts();
    Frame_encode_unconfirmedUp_withOpts_upCount();
    
    exit(EXIT_SUCCESS);
}
