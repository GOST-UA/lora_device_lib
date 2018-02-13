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
 * 
 * */

#include <ruby.h>
#include <stddef.h>
#include <assert.h>

#include "lora_frame.h"

static VALUE cLDL;
static VALUE cFrame;
static VALUE cJoinReq;
static VALUE cJoinAccept;
static VALUE cDataFrame;
static VALUE cConfirmedUp;
static VALUE cUnconfirmedUp;
static VALUE cConfirmedDown;
static VALUE cUnconfirmedDown;
static VALUE cKey;
static VALUE cError;
static VALUE cEUI64;

static const struct {
        
    VALUE *klass;
    enum lora_frame_type type;

} map[] = {
    {
        .klass = &cJoinReq,
        .type = FRAME_TYPE_JOIN_REQ
    },
    {
        .klass = &cJoinAccept,
        .type = FRAME_TYPE_JOIN_ACCEPT
    },
    {
        .klass = &cUnconfirmedUp,
        .type = FRAME_TYPE_DATA_UNCONFIRMED_UP
    },
    {
        .klass = &cUnconfirmedDown,
        .type = FRAME_TYPE_DATA_UNCONFIRMED_DOWN
    },
    {
        .klass = &cConfirmedUp,
        .type = FRAME_TYPE_DATA_CONFIRMED_UP
    },
    {
        .klass = &cConfirmedDown,
        .type = FRAME_TYPE_DATA_CONFIRMED_DOWN
    }        
};

static const uint8_t default_key[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

/* static functions ***************************************************/

static VALUE _decode(int argc, VALUE *argv, VALUE self);

static VALUE frameTypeToKlass(enum lora_frame_type type);
static bool klassToFrameType(VALUE klass, enum lora_frame_type *type);

static VALUE _encode_data(VALUE self);
static VALUE _encode_join_accept(VALUE self);
static VALUE _encode_join_req(VALUE self);


/* functions **********************************************************/

void Init_ext_frame(void)
{
    cLDL = rb_define_module("LDL");
    cFrame = rb_define_class_under(cLDL, "Frame", rb_cObject);
    
    cKey = rb_const_get(cLDL, rb_intern("Key"));
    cError = rb_const_get(cLDL, rb_intern("Error"));
    
    cEUI64 = rb_const_get(cLDL, rb_intern("EUI64"));
    
    cJoinReq = rb_define_class_under(cLDL, "JoinReq", cFrame);
    cJoinAccept = rb_define_class_under(cLDL, "JoinAccept", cFrame);
    cDataFrame = rb_define_class_under(cLDL, "DataFrame", cFrame);
    cUnconfirmedUp = rb_define_class_under(cLDL, "UnconfirmedUp", cDataFrame);
    cConfirmedUp = rb_define_class_under(cLDL, "ConfirmedUp", cDataFrame);
    cUnconfirmedDown = rb_define_class_under(cLDL, "UnconfirmedDown", cDataFrame);
    cConfirmedDown = rb_define_class_under(cLDL, "ConfirmedDown", cDataFrame);
    
    rb_define_singleton_method(cFrame, "decode", _decode, -1);
    
    rb_define_method(cJoinReq, "encode", _encode_join_req, 0);
    rb_define_method(cJoinAccept, "encode", _encode_join_accept, 0);
    rb_define_method(cDataFrame, "encode", _encode_data, 0);
}

/* static functions ***************************************************/

static VALUE _decode(int argc, VALUE *argv, VALUE self)
{
    struct lora_frame f;
    
    VALUE nwkSKey;
    VALUE appSKey;
    VALUE appKey;
    VALUE param;
    VALUE klass;
    VALUE input;
    VALUE keys;
    VALUE defaultKey;
    
    {
        VALUE args[] = {rb_str_new((char *)default_key, sizeof(default_key)-1U)};
        defaultKey = rb_class_new_instance(sizeof(args)/sizeof(*args), args, cKey);
    }
    
    (void)rb_scan_args(argc, argv, "10:", &input, &keys);
    
    if(keys == Qnil){
        
        keys = rb_hash_new();
    }
    
    if(rb_hash_aref(keys, ID2SYM(rb_intern("nwkSKey"))) != Qnil){
        
        nwkSKey = rb_hash_aref(keys, ID2SYM(rb_intern("nwkSKey")));
    }
    else{
        
        nwkSKey = defaultKey;
    }
    
    if(rb_hash_aref(keys, ID2SYM(rb_intern("appSKey"))) != Qnil){
        
        appSKey = rb_hash_aref(keys, ID2SYM(rb_intern("appSKey")));
    }
    else{
        
        appSKey = defaultKey;
    }
    
    if(rb_hash_aref(keys, ID2SYM(rb_intern("appKey"))) != Qnil){
        
        appKey = rb_hash_aref(keys, ID2SYM(rb_intern("appKey")));
    }
    else{
        
        appKey = defaultKey;
    }
     
    if(rb_obj_is_kind_of(nwkSKey, cKey) != Qtrue){
        
        rb_raise(rb_eTypeError, ":nwkSKey parameter must be kind_of Key");
    }
    if(rb_obj_is_kind_of(appSKey, cKey) != Qtrue){
        
        rb_raise(rb_eTypeError, ":appSKey parameter must be kind_of Key");
    }
    if(rb_obj_is_kind_of(appKey, cKey) != Qtrue){
        
        rb_raise(rb_eTypeError, ":appKey parameter must be kind_of Key");
    }
    
    VALUE mutable = rb_str_new(RSTRING_PTR(input), RSTRING_LEN(input));
        
    if(Frame_decode(RSTRING_PTR(rb_funcall(appKey, rb_intern("value"), 0)), RSTRING_PTR(rb_funcall(nwkSKey, rb_intern("value"), 0)), RSTRING_PTR(rb_funcall(appSKey, rb_intern("value"), 0)), RSTRING_PTR(mutable), RSTRING_LEN(mutable), &f)){
    
        if(!f.valid){
            
            rb_funcall(rb_cObject, rb_intern("raise"), 1, rb_funcall(cError, rb_intern("new"), 1, rb_str_new2("invalid MIC")));
        }
    
    }
    else{
        
        rb_funcall(rb_cObject, rb_intern("raise"), 1, rb_funcall(cError, rb_intern("new"), 1, rb_str_new2("bad frame")));
    }
    
    param = rb_hash_new();

    klass = frameTypeToKlass(f.type);
    
    switch(f.type){
    default:
    case FRAME_TYPE_JOIN_REQ:
    {
        rb_hash_aset(param, ID2SYM(rb_intern("appKey")), appKey);
        
        rb_hash_aset(param, ID2SYM(rb_intern("appEUI")), rb_funcall(cEUI64, rb_intern("new"), 1, rb_str_new((char *)f.fields.joinRequest.appEUI, sizeof(f.fields.joinRequest.appEUI))));
        rb_hash_aset(param, ID2SYM(rb_intern("devEUI")), rb_funcall(cEUI64, rb_intern("new"), 1, rb_str_new((char *)f.fields.joinRequest.devEUI, sizeof(f.fields.joinRequest.devEUI))));
        rb_hash_aset(param, ID2SYM(rb_intern("devNonce")), UINT2NUM(f.fields.joinRequest.devNonce));
    }
        break;
    case FRAME_TYPE_JOIN_ACCEPT:
    {
        rb_hash_aset(param, ID2SYM(rb_intern("appKey")), appKey);
        
        rb_hash_aset(param, ID2SYM(rb_intern("appNonce")), UINT2NUM(f.fields.joinAccept.appNonce));
        rb_hash_aset(param, ID2SYM(rb_intern("netID")), UINT2NUM(f.fields.joinAccept.netID));
        rb_hash_aset(param, ID2SYM(rb_intern("devAddr")), UINT2NUM(f.fields.joinAccept.devAddr));
        rb_hash_aset(param, ID2SYM(rb_intern("rx1DataRateOffset")), UINT2NUM(f.fields.joinAccept.rx1DataRateOffset));
        rb_hash_aset(param, ID2SYM(rb_intern("rx2DataRate")), UINT2NUM(f.fields.joinAccept.rx2DataRate));
        rb_hash_aset(param, ID2SYM(rb_intern("rxDelay")), UINT2NUM(f.fields.joinAccept.rxDelay));
        
        
        if(f.fields.joinAccept.cfListPresent){
        
            VALUE cfList = rb_ary_new();
            size_t i;
            
            for(i=0U; i < sizeof(f.fields.joinAccept.cfList)/sizeof(*f.fields.joinAccept.cfList); i++){
                
                rb_ary_push(cfList, f.fields.joinAccept.cfList[i]);
            }
            
            rb_hash_aset(param, ID2SYM(rb_intern("cfList")), cfList);
        }
        
    }
        break;
    case FRAME_TYPE_DATA_UNCONFIRMED_UP:
    case FRAME_TYPE_DATA_UNCONFIRMED_DOWN:
    case FRAME_TYPE_DATA_CONFIRMED_UP:
    case FRAME_TYPE_DATA_CONFIRMED_DOWN:
    {    
        rb_hash_aset(param, ID2SYM(rb_intern("nwkSKey")), nwkSKey);
        rb_hash_aset(param, ID2SYM(rb_intern("appSKey")), appSKey);
        
        rb_hash_aset(param, ID2SYM(rb_intern("counter")), UINT2NUM(f.fields.data.counter));      
        rb_hash_aset(param, ID2SYM(rb_intern("devAddr")), UINT2NUM(f.fields.data.devAddr));      
        
        rb_hash_aset(param, ID2SYM(rb_intern("ack")), f.fields.data.ack ? Qtrue : Qfalse);      
        rb_hash_aset(param, ID2SYM(rb_intern("adr")), f.fields.data.adr ? Qtrue : Qfalse);            
        rb_hash_aset(param, ID2SYM(rb_intern("adrAckReq")), f.fields.data.adrAckReq ? Qtrue : Qfalse);            
        rb_hash_aset(param, ID2SYM(rb_intern("pending")), f.fields.data.pending ? Qtrue : Qfalse);            
        
        rb_hash_aset(param, ID2SYM(rb_intern("opts")), (f.fields.data.optsLen > 0U) ? rb_str_new((char *)f.fields.data.opts, f.fields.data.optsLen) : Qnil);            
        rb_hash_aset(param, ID2SYM(rb_intern("data")), (f.fields.data.dataLen > 0U) ? rb_str_new((char *)f.fields.data.data, f.fields.data.dataLen) : Qnil);            
        rb_hash_aset(param, ID2SYM(rb_intern("port")), (f.fields.data.dataLen > 0U) ? UINT2NUM(f.fields.data.port) : Qnil);            
    }
        break;
    }
    
    VALUE args[] = {param};
    
    return rb_class_new_instance(sizeof(args)/sizeof(*args),args,klass);
}

static VALUE frameTypeToKlass(enum lora_frame_type type)
{
    return *map[type].klass;
}

static bool klassToFrameType(VALUE klass, enum lora_frame_type *type)
{
    size_t i;
    bool retval = false;
    
    for(i=0U; i < sizeof(map)/sizeof(*map); i++){
        
        if(*map[i].klass == klass){
            
            *type = map[i].type;
            retval = true;
            break;
        }
    }
    
    return retval;
}

static VALUE _encode_join_req(VALUE self)
{
    uint8_t out[UINT8_MAX];
    uint8_t len;
    
    struct lora_frame_join_request f;
    
    VALUE appKey;
    VALUE retval;
    
    VALUE appEUI;
    VALUE devEUI;
    VALUE devNonce;
    
    (void)memset(&f, 0, sizeof(f));
    
    appKey = rb_iv_get(self, "@appKey");    
    appEUI = rb_iv_get(self, "@appEUI");
    devEUI = rb_iv_get(self, "@devEUI");
    devNonce = rb_iv_get(self, "@devNonce");
        
    f.devNonce = NUM2UINT(devNonce);            
    (void)memcpy(f.appEUI, RSTRING_PTR(appEUI), sizeof(f.appEUI));
    (void)memcpy(f.appEUI, RSTRING_PTR(devEUI), sizeof(f.devEUI));
            
    len = Frame_putJoinRequest(RSTRING_PTR(rb_funcall(appKey, rb_intern("value"), 0)), &f, out, sizeof(out));
    
    assert(len != 0U);
    
    retval = rb_str_new((char *)out, len);
        
    return retval;
}

static VALUE _encode_join_accept(VALUE self)
{    
    uint8_t out[UINT8_MAX];
    uint8_t len;
    
    struct lora_frame_join_accept f;
    
    VALUE appKey;
    VALUE retval;
    VALUE rx1DataRateOffset;
    VALUE rx2DataRate;
    VALUE rxDelay;
    VALUE cfList;
    VALUE appNonce;
    VALUE netID;
    VALUE devAddr;
    
    (void)memset(&f, 0, sizeof(f));
    
    appKey = rb_iv_get(self, "@appKey");
    rx1DataRateOffset = rb_iv_get(self, "@rx1DataRateOffset");
    rx2DataRate = rb_iv_get(self, "@rx2DataRate");
    rxDelay = rb_iv_get(self, "@rxDelay");
    appNonce = rb_iv_get(self, "@appNonce");
    netID = rb_iv_get(self, "@netID");
    devAddr = rb_iv_get(self, "@devAddr");
    cfList = rb_iv_get(self, "@cfList");
    
    f.rx1DataRateOffset = NUM2UINT(rx1DataRateOffset);
    f.rx2DataRate = NUM2UINT(rx2DataRate);
    f.rxDelay = NUM2UINT(rxDelay);
    f.appNonce = NUM2UINT(appNonce);
    f.netID = NUM2UINT(netID);
    f.devAddr = NUM2UINT(devAddr);
    f.cfListPresent = ( cfList != Qnil) ? true : false;
    
    if(f.cfListPresent){
        
        f.cfList[0] = rb_ary_entry(cfList, 0);
        f.cfList[1] = rb_ary_entry(cfList, 1);
        f.cfList[2] = rb_ary_entry(cfList, 2);
        f.cfList[3] = rb_ary_entry(cfList, 3);
        f.cfList[4] = rb_ary_entry(cfList, 4);
    }
    
    len = Frame_putJoinAccept(RSTRING_PTR(rb_funcall(appKey, rb_intern("value"), 0)), &f, out, sizeof(out));
    
    assert(len != 0U);
    
    retval = rb_str_new((char *)out, len);        
    
    return retval;
}

static VALUE _encode_data(VALUE self)
{
    enum lora_frame_type type;
    uint8_t out[UINT8_MAX];
    
    uint8_t len;
    
    struct lora_frame_data f;
        
    VALUE nwkSKey;
    VALUE appSKey;
    VALUE ack;
    VALUE adr;
    VALUE adrAckReq;
    VALUE pending;
    VALUE opts;
    VALUE data;
    VALUE port;
    VALUE devAddr;
    VALUE counter;
    VALUE retval;
    
    (void)memset(&f, 0, sizeof(f));
    
    (void)klassToFrameType(rb_funcall(self, rb_intern("class"), 0), &type);

    
    nwkSKey = rb_funcall(rb_iv_get(self, "@nwkSKey"), rb_intern("value"), 0);
    appSKey = rb_funcall(rb_iv_get(self, "@appSKey"), rb_intern("value"), 0);
        
    data = rb_iv_get(self, "@data");
    port = rb_iv_get(self, "@port");
    opts = rb_iv_get(self, "@opts");
    
    ack = rb_iv_get(self, "@ack");
    adr = rb_iv_get(self, "@adr");
    adrAckReq = rb_iv_get(self, "@adrAckReq");
    pending = rb_iv_get(self, "@pending");
    
    devAddr = rb_iv_get(self, "@devAddr");
    
    counter = rb_iv_get(self, "@counter");
    
    f.devAddr = NUM2UINT(devAddr);
    f.counter = NUM2UINT(counter);
    f.ack = (ack == Qtrue) ? true : false;
    f.adr = (adr == Qtrue) ? true : false;
    f.adrAckReq = (adrAckReq == Qtrue) ? true : false;
    f.pending = (pending == Qtrue) ? true : false;
    
    f.opts = (opts != Qnil) ? (uint8_t *)RSTRING_PTR(opts) : NULL;
    f.optsLen = (opts != Qnil) ? RSTRING_LEN(opts) : 0U;             
    
    f.data = (data != Qnil) ? (uint8_t *)RSTRING_PTR(data) : NULL;
    f.dataLen = (data != Qnil) ? RSTRING_LEN(data) : 0U;
    f.port = (port != Qnil) ? NUM2UINT(port) : 0U;

    len = Frame_putData(type, RSTRING_PTR(nwkSKey), RSTRING_PTR(appSKey), &f, out, sizeof(out));
    
    assert(len != 0U);
    
    retval = rb_str_new((char *)out, len);
    
    return retval;
}
