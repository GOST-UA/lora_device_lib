#include <ruby.h>
#include <stddef.h>
#include <assert.h>

#include "lora_frame.h"

static VALUE cLoraDeviceLib;
static VALUE cFrame;
static VALUE cJoinReq;
static VALUE cJoinAccept;
static VALUE cDataFrame;
static VALUE cConfirmedUp;
static VALUE cUnconfirmedUp;
static VALUE cConfirmedDown;
static VALUE cUnconfirmedDown;

static const struct {
        
    VALUE *klass;
    enum message_type type;

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

/* static functions ***************************************************/

static VALUE _decode(int argc, VALUE *argv, VALUE self);

static VALUE frameTypeToKlass(enum message_type type);
static bool klassToFrameType(VALUE klass, enum message_type *type);

static VALUE _encode_data(VALUE self);
static VALUE _encode_join_accept(VALUE self);
static VALUE _encode_join_req(VALUE self);

static VALUE _initialize_joinReq(int argc, VALUE *argv, VALUE self);
static VALUE _initialize_joinAccept(int argc, VALUE *argv, VALUE self);

static VALUE _initialize_data(int argc, VALUE *argv, VALUE self);


/* functions **********************************************************/

void Init_ext_frame(void)
{
    cLoraDeviceLib = rb_define_module("LoraDeviceLib");
    cFrame = rb_define_class_under(cLoraDeviceLib, "Frame", rb_cObject);
    
    cJoinReq = rb_define_class_under(cLoraDeviceLib, "JoinReq", cFrame);
    cJoinAccept = rb_define_class_under(cLoraDeviceLib, "JoinAccept", cFrame);
    cDataFrame = rb_define_class_under(cLoraDeviceLib, "DataFrame", cFrame);
    cUnconfirmedUp = rb_define_class_under(cLoraDeviceLib, "UnconfirmedUp", cDataFrame);
    cConfirmedUp = rb_define_class_under(cLoraDeviceLib, "ConfirmedUp", cDataFrame);
    cUnconfirmedDown = rb_define_class_under(cLoraDeviceLib, "UnconfirmedDown", cDataFrame);
    cConfirmedDown = rb_define_class_under(cLoraDeviceLib, "ConfirmedDown", cDataFrame);
    
    rb_define_singleton_method(cFrame, "decode", _decode, -1);
    
    rb_define_method(cJoinReq, "encode", _encode_join_req, 0);
    rb_define_method(cJoinAccept, "encode", _encode_join_accept, 0);
    rb_define_method(cDataFrame, "encode", _encode_data, 0);
    
    rb_define_method(cJoinReq, "initialize", _initialize_joinReq, -1);
    rb_define_method(cJoinAccept, "initialize", _initialize_joinAccept, -1);
    
    rb_define_method(cDataFrame, "initialize", _initialize_data, -1);
}

/* static functions ***************************************************/

static VALUE _decode(int argc, VALUE *argv, VALUE self)
{
    struct lora_frame f;
    enum lora_frame_result result;
    
    VALUE nwkSKey;
    VALUE appSKey;
    VALUE appKey;
    VALUE param;
    VALUE klass;
    VALUE input;
    VALUE keys;
    
    (void)rb_scan_args(argc, argv, "10:", &input, &keys);
    
    nwkSKey = rb_hash_aref(keys, ID2SYM(rb_intern("nwkSKey")));
    appSKey = rb_hash_aref(keys, ID2SYM(rb_intern("appSKey")));
    appKey = rb_hash_aref(keys, ID2SYM(rb_intern("appKey")));
    
    result = Frame_decode(RSTRING_PTR(appKey), RSTRING_PTR(nwkSKey), RSTRING_PTR(appSKey), RSTRING_PTR(input), RSTRING_LEN(input), &f);
    
    if(result == LORA_FRAME_BAD){
        
        //raise exception
    }
    
    param = rb_hash_new();
    
    rb_hash_aset(param, ID2SYM(rb_intern("result")), (result == LORA_FRAME_OK) ? ID2SYM(rb_intern(":ok")) : ID2SYM(rb_intern(":mic_failure")));

    klass = frameTypeToKlass(f.type);
    
    rb_hash_aset(param, ID2SYM(rb_intern("original")), input);      
    
    switch(f.type){
    default:
    case FRAME_TYPE_JOIN_REQ:
    {
        rb_hash_aset(param, ID2SYM(rb_intern("appKey")), appKey);
        
        rb_hash_aset(param, ID2SYM(rb_intern("appEUI")), rb_str_new((char *)f.fields.joinRequest.appEUI, sizeof(f.fields.joinRequest.appEUI)));
        rb_hash_aset(param, ID2SYM(rb_intern("devEUI")), rb_str_new((char *)f.fields.joinRequest.devEUI, sizeof(f.fields.joinRequest.devEUI)));
        rb_hash_aset(param, ID2SYM(rb_intern("devNonce")), rb_str_new((char *)f.fields.joinRequest.devNonce, sizeof(f.fields.joinRequest.devNonce)));    
    }
        break;
    case FRAME_TYPE_JOIN_ACCEPT:
    {
        rb_hash_aset(param, ID2SYM(rb_intern("appKey")), appKey);
        
        rb_hash_aset(param, ID2SYM(rb_intern("appNonce")), rb_str_new((char *)f.fields.joinAccept.appNonce, sizeof(f.fields.joinAccept.appNonce)));
        rb_hash_aset(param, ID2SYM(rb_intern("netID")), rb_str_new((char *)f.fields.joinAccept.netID, sizeof(f.fields.joinAccept.netID)));
        rb_hash_aset(param, ID2SYM(rb_intern("devAddr")), rb_str_new((char *)f.fields.joinAccept.devAddr, sizeof(f.fields.joinAccept.devAddr)));
        rb_hash_aset(param, ID2SYM(rb_intern("rx1DataRateOffset")), UINT2NUM(f.fields.joinAccept.rx1DataRateOffset));
        rb_hash_aset(param, ID2SYM(rb_intern("rx2DataRate")), UINT2NUM(f.fields.joinAccept.rx2DataRate));
        rb_hash_aset(param, ID2SYM(rb_intern("rxDelay")), UINT2NUM(f.fields.joinAccept.rxDelay));
        rb_hash_aset(param, ID2SYM(rb_intern("cfList")), (f.fields.joinAccept.cfListLen > 0U) ? rb_str_new((char *)f.fields.joinAccept.cfList, sizeof(f.fields.joinAccept.cfListLen)) : Qnil);
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
        rb_hash_aset(param, ID2SYM(rb_intern("devAddr")), rb_str_new((char *)f.fields.data.devAddr, sizeof(f.fields.data.devAddr)));      
        
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
    
    return rb_class_new_instance(sizeof(args),args,klass);
}

static VALUE _initialize_joinReq(int argc, VALUE *argv, VALUE self)
{
    VALUE opts;
    
    (void)rb_scan_args(argc, argv, "00:", &opts);
    
    rb_iv_set(self, "@appKey", rb_hash_aref(opts, ID2SYM(rb_intern("appKey"))));
    rb_iv_set(self, "@appEUI", rb_hash_aref(opts, ID2SYM(rb_intern("appEUI"))));
    rb_iv_set(self, "@devEUI", rb_hash_aref(opts, ID2SYM(rb_intern("devEUI"))));
    rb_iv_set(self, "@devNonce", rb_hash_aref(opts, ID2SYM(rb_intern("devNonce"))));
   
    return self;
}

static VALUE _initialize_joinAccept(int argc, VALUE *argv, VALUE self)
{
    VALUE opts;
    
    (void)rb_scan_args(argc, argv, "00:", &opts);
    
    rb_iv_set(self, "@appKey", rb_hash_aref(opts, ID2SYM(rb_intern("appKey"))));
    rb_iv_set(self, "@netID", rb_hash_aref(opts, ID2SYM(rb_intern("netID"))));
    rb_iv_set(self, "@devAddr", rb_hash_aref(opts, ID2SYM(rb_intern("devAddr"))));
    rb_iv_set(self, "@rx1DataRateOffset", rb_hash_aref(opts, ID2SYM(rb_intern("rx1DataRateOffset"))));
    rb_iv_set(self, "@rx2DataRate", rb_hash_aref(opts, ID2SYM(rb_intern("rx2DataRate"))));
    rb_iv_set(self, "@rxDelay", rb_hash_aref(opts, ID2SYM(rb_intern("rxDelay"))));
    rb_iv_set(self, "@cfList", rb_hash_aref(opts, ID2SYM(rb_intern("cfList"))));
    
    return self;
}

static VALUE _initialize_data(int argc, VALUE *argv, VALUE self)
{
    VALUE opts;
    
    (void)rb_scan_args(argc, argv, "00:", &opts);
    
    rb_iv_set(self, "@nwkSKey", rb_hash_aref(opts, ID2SYM(rb_intern("nwkSKey"))));
    rb_iv_set(self, "@appSKey", rb_hash_aref(opts, ID2SYM(rb_intern("appSKey"))));
    rb_iv_set(self, "@counter", rb_hash_aref(opts, ID2SYM(rb_intern("counter"))));
    rb_iv_set(self, "@devAddr", rb_hash_aref(opts, ID2SYM(rb_intern("devAddr"))));
    
    rb_iv_set(self, "@ack", rb_hash_aref(opts, ID2SYM(rb_intern("ack"))));
    rb_iv_set(self, "@adr", rb_hash_aref(opts, ID2SYM(rb_intern("adr"))));
    rb_iv_set(self, "@adrAckReq", rb_hash_aref(opts, ID2SYM(rb_intern("adrAckReq"))));
    rb_iv_set(self, "@pending", rb_hash_aref(opts, ID2SYM(rb_intern("pending"))));
    
    rb_iv_set(self, "@opts", rb_hash_aref(opts, ID2SYM(rb_intern("opts"))));
    rb_iv_set(self, "@data", rb_hash_aref(opts, ID2SYM(rb_intern("data"))));
    rb_iv_set(self, "@port", rb_hash_aref(opts, ID2SYM(rb_intern("port"))));
    
    return self;
}

static VALUE frameTypeToKlass(enum message_type type)
{
    return *map[type].klass;
}

static bool klassToFrameType(VALUE klass, enum message_type *type)
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
    
    struct lora_frame f;
    
    VALUE appKey;
    VALUE original;
    VALUE retval;
    
    VALUE appEUI;
    VALUE devEUI;
    VALUE devNonce;
    
    (void)memset(&f, 0, sizeof(f));
    
    appKey = rb_iv_get(self, "@appKey");    
    original = rb_iv_get(self, "@original");
    appEUI = rb_iv_get(self, "@appEUI");
    devEUI = rb_iv_get(self, "@devEUI");
    devNonce = rb_iv_get(self, "@devNonce");
    
    if(original != Qnil){
        
        retval = original;
    }
    else{

        f.type = FRAME_TYPE_JOIN_REQ;
        (void)memcpy(f.fields.joinRequest.devNonce, RSTRING_PTR(devNonce), sizeof(f.fields.joinRequest.devNonce));            
        (void)memcpy(f.fields.joinRequest.appEUI, RSTRING_PTR(appEUI), sizeof(f.fields.joinRequest.appEUI));
        (void)memcpy(f.fields.joinRequest.appEUI, RSTRING_PTR(devEUI), sizeof(f.fields.joinRequest.devEUI));
                
        len = Frame_encode(RSTRING_PTR(appKey), &f, out, sizeof(out));
        
        assert(len != 0U);
        
        retval = rb_str_new((char *)out, len);
    }
    
    return retval;
}

static VALUE _encode_join_accept(VALUE self)
{    
    uint8_t out[UINT8_MAX];
    uint8_t len;
    
    struct lora_frame f;
    
    VALUE appKey;
    VALUE original;
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
    original = rb_iv_get(self, "@original");
    rx1DataRateOffset = rb_iv_get(self, "@rx1DataRateOffset");
    rx2DataRate = rb_iv_get(self, "@rx2DataRate");
    rxDelay = rb_iv_get(self, "@rxDelay");
    cfList = rb_iv_get(self, "@cfList");
    appNonce = rb_iv_get(self, "@appNonce");
    netID = rb_iv_get(self, "@netID");
    devAddr = rb_iv_get(self, "@devAddr");
    
    if(original != Qnil){
        
        retval = original;
    }
    else{
        
        assert(RSTRING_LEN(cfList) <= sizeof(f.fields.joinAccept.cfList));
        
        f.type = FRAME_TYPE_JOIN_REQ;
        f.fields.joinAccept.rx1DataRateOffset = NUM2UINT(rx1DataRateOffset);
        f.fields.joinAccept.rx2DataRate = NUM2UINT(rx2DataRate);
        f.fields.joinAccept.rxDelay = NUM2UINT(rxDelay);
        (void)memcpy(f.fields.joinAccept.cfList, RSTRING_PTR(cfList), RSTRING_LEN(cfList));
        (void)memcpy(f.fields.joinAccept.appNonce, RSTRING_PTR(appNonce), sizeof(f.fields.joinAccept.appNonce));
        (void)memcpy(f.fields.joinAccept.netID, RSTRING_PTR(netID), sizeof(f.fields.joinAccept.netID));
        (void)memcpy(f.fields.joinAccept.devAddr, RSTRING_PTR(devAddr), sizeof(f.fields.joinAccept.devAddr));
        
        len = Frame_encode(RSTRING_PTR(appKey), &f, out, sizeof(out));
        
        assert(len != 0U);
        
        retval = rb_str_new((char *)out, len);        
    }
        
    return retval;
}

static VALUE _encode_data(VALUE self)
{
    enum message_type type;
    
    uint8_t out[UINT8_MAX];
    uint8_t len;
    
    struct lora_frame f;
    
    VALUE nwkSKey;
    VALUE appSKey;
    VALUE original;
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
    
    original = rb_iv_get(self, "@original");
    
    if(original != Qnil){
        
        retval = original;
    }
    else{
    
        nwkSKey = rb_iv_get(self, "@nwkSKey");
        appSKey = rb_iv_get(self, "@appSKey");
            
        data = rb_iv_get(self, "@payload");
        port = rb_iv_get(self, "@port");
        opts = rb_iv_get(self, "@opts");
        
        ack = rb_iv_get(self, "@ack");
        adr = rb_iv_get(self, "@adr");
        adrAckReq = rb_iv_get(self, "@adrAckReq");
        pending = rb_iv_get(self, "@pending");
        
        devAddr = rb_iv_get(self, "@devAddr");
        
        counter = rb_iv_get(self, "@counter");
        
        f.type = type;
        (void)memcpy(f.fields.data.devAddr, RSTRING_PTR(devAddr), RSTRING_LEN(devAddr));
        f.fields.data.counter = NUM2UINT(counter);
        f.fields.data.ack = (ack == Qtrue) ? true : false;
        f.fields.data.adr = (adr == Qtrue) ? true : false;
        f.fields.data.adrAckReq = (adrAckReq == Qtrue) ? true : false;
        f.fields.data.pending = (pending == Qtrue) ? true : false;
        
        f.fields.data.opts = (opts != Qnil) ? (uint8_t *)RSTRING_PTR(opts) : NULL;
        f.fields.data.optsLen = (opts != Qnil) ? RSTRING_LEN(opts) : 0U;             
        
        f.fields.data.data = (data != Qnil) ? (uint8_t *)RSTRING_LEN(data) : NULL;
        f.fields.data.dataLen = (data != Qnil) ? RSTRING_LEN(data) : 0U;
        f.fields.data.port = (port != Qnil) ? NUM2UINT(port) : 0U;
            
        len = Frame_encode((f.fields.data.port == 0) ? RSTRING_PTR(nwkSKey) : RSTRING_PTR(appSKey), &f, out, sizeof(out));
        
        assert(len != 0U);
        
        retval = rb_str_new((char *)out, len);
    }
    
    return retval;
}
