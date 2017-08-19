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
static VALUE cKey;
static VALUE cError;
static VALUE cEUI64;

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

static const uint8_t default_key[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

/* static functions ***************************************************/

static VALUE _decode(int argc, VALUE *argv, VALUE self);

static VALUE frameTypeToKlass(enum message_type type);
static bool klassToFrameType(VALUE klass, enum message_type *type);

static VALUE _encode_data(VALUE self);
static VALUE _encode_join_accept(VALUE self);
static VALUE _encode_join_req(VALUE self);


/* functions **********************************************************/

void Init_ext_frame(void)
{
    cLoraDeviceLib = rb_define_module("LoraDeviceLib");
    cFrame = rb_define_class_under(cLoraDeviceLib, "Frame", rb_cObject);
    
    cKey = rb_const_get(cLoraDeviceLib, rb_intern("Key"));
    cError = rb_const_get(cLoraDeviceLib, rb_intern("Error"));
    
    cEUI64 = rb_const_get(cLoraDeviceLib, rb_intern("EUI64"));
    
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
        
    result = Frame_decode(RSTRING_PTR(rb_funcall(appKey, rb_intern("value"), 0)), RSTRING_PTR(rb_funcall(nwkSKey, rb_intern("value"), 0)), RSTRING_PTR(rb_funcall(appSKey, rb_intern("value"), 0)), RSTRING_PTR(input), RSTRING_LEN(input), &f);
    
    if(result == LORA_FRAME_BAD){
        
        rb_funcall(rb_cObject, rb_intern("raise"), 1, rb_funcall(cError, rb_intern("new"), 1, rb_str_new2("bad frame")));
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
        f.fields.joinRequest.devNonce = NUM2UINT(devNonce);            
        (void)memcpy(f.fields.joinRequest.appEUI, RSTRING_PTR(appEUI), sizeof(f.fields.joinRequest.appEUI));
        (void)memcpy(f.fields.joinRequest.appEUI, RSTRING_PTR(devEUI), sizeof(f.fields.joinRequest.devEUI));
                
        len = Frame_encode(RSTRING_PTR(rb_funcall(appKey, rb_intern("value"), 0)), &f, out, sizeof(out));
        
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
        f.fields.joinAccept.appNonce = NUM2UINT(appNonce);
        f.fields.joinAccept.netID = NUM2UINT(netID);
        f.fields.joinAccept.devAddr = NUM2UINT(devAddr);
        
        len = Frame_encode(RSTRING_PTR(rb_funcall(appKey, rb_intern("value"), 0)), &f, out, sizeof(out));
        
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
        
        f.type = type;
        
        f.fields.data.devAddr = NUM2UINT(devAddr);
        f.fields.data.counter = NUM2UINT(counter);
        f.fields.data.ack = (ack == Qtrue) ? true : false;
        f.fields.data.adr = (adr == Qtrue) ? true : false;
        f.fields.data.adrAckReq = (adrAckReq == Qtrue) ? true : false;
        f.fields.data.pending = (pending == Qtrue) ? true : false;
        
        f.fields.data.opts = (opts != Qnil) ? (uint8_t *)RSTRING_PTR(opts) : NULL;
        f.fields.data.optsLen = (opts != Qnil) ? RSTRING_LEN(opts) : 0U;             
        
        f.fields.data.data = (data != Qnil) ? (uint8_t *)RSTRING_PTR(data) : NULL;
        f.fields.data.dataLen = (data != Qnil) ? RSTRING_LEN(data) : 0U;
        f.fields.data.port = (port != Qnil) ? NUM2UINT(port) : 0U;

        const uint8_t k[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

        len = Frame_encode(k, &f, out, sizeof(out));
        
        assert(len != 0U);
        
        retval = rb_str_new((char *)out, len);
    }
    
    return retval;
}
