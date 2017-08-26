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

#include "lora_device_lib.h"

static const uint8_t default_key[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
static const uint8_t default_eui[] = "\x00\x00\x00\x00\x00\x00\x00\x00";

static VALUE cLDL;
static VALUE cExtMAC;
static VALUE cEUI64;
static VALUE cKey;
static VALUE cError;
static VALUE cRadio;

static VALUE _ldl_initialize(int argc, VALUE *argv, VALUE self);
static VALUE _ldl_initialize_copy(VALUE copy, VALUE orig);
static VALUE _ldl_alloc(VALUE klass);

static VALUE _ldl_personalize(VALUE self, VALUE devAddr, VALUE nwkSKey, VALUE appSKey);
static VALUE _ldl_tick(VALUE self);
static VALUE _ldl_addChannel(VALUE self, VALUE freq, VALUE chIndex);
static VALUE _ldl_removeChannel(VALUE self, VALUE chIndex);
static VALUE _ldl_maskChannel(VALUE self, VALUE chIndex);
static VALUE _ldl_unmaskChannel(VALUE self, VALUE chIndex);
static VALUE _ldl_setRateAndPower(VALUE self, VALUE rate, VALUE power);
static VALUE _ldl_join(int argc, VALUE *argv, VALUE self);
static VALUE _ldl_send_unconfirmed(int argc, VALUE *argv, VALUE self);
static VALUE _ldl_send_confirmed(int argc, VALUE *argv, VALUE self);
static VALUE io_event(VALUE self, VALUE event, VALUE time);

static void _response(void *receiver, enum lora_mac_response_type type, const union lora_mac_response_arg *arg);

static VALUE bw_to_symbol(enum lora_signal_bandwidth bw);
static VALUE sf_to_symbol(enum lora_spreading_factor sf);
static VALUE cr_to_symbol(enum lora_coding_rate cr);

uint64_t System_getTime(void)
{
    return NUM2ULL(rb_funcall(rb_const_get(cLDL, rb_intern("SystemTime")), rb_intern("time"), 0));
}

void System_usleep(uint32_t interval)
{
}

void System_rand(uint8_t *data, size_t len)
{
    VALUE result = rb_random_bytes(rb_str_new2(""), len);
    
    (void)memcpy(data, RSTRING_PTR(result), len);
}

void System_atomic_setPtr(void **receiver, void *value)
{
    *receiver = value;  //fixme
}

void System_getAppEUI(void *owner, uint8_t *eui)
{
    VALUE self = (VALUE)owner;
    
    VALUE appEUI = rb_iv_get(self, "@appEUI");
    
    (void)memcpy(eui, RSTRING_PTR(rb_funcall(appEUI, rb_intern("bytes"), 0)), sizeof(default_eui));
}

void System_getDevEUI(void *owner, uint8_t *eui)
{
    VALUE self = (VALUE)owner;    
    VALUE devEUI = rb_iv_get(self, "@devEUI");
    
    (void)memcpy(eui, RSTRING_PTR(rb_funcall(devEUI, rb_intern("bytes"), 0)), sizeof(default_eui));
}

void System_getAppKey(void *owner, uint8_t *key)
{
    VALUE self = (VALUE)owner;    
    VALUE appKey = rb_iv_get(self, "@appKey");
    
    (void)memcpy(key, RSTRING_PTR(rb_funcall(appKey, rb_intern("value"), 0)), sizeof(default_key));
}

void Init_ext_mac(void)
{
    cLDL = rb_define_module("LDL");
    
    cExtMAC = rb_define_class_under(cLDL, "ExtMAC", rb_cObject);
    rb_define_alloc_func(cExtMAC, _ldl_alloc);
    
    rb_define_method(cExtMAC, "initialize", _ldl_initialize, 1);
    rb_define_method(cExtMAC, "initialize_copy", _ldl_initialize_copy, 1);
    
    rb_define_method(cExtMAC, "personalize", _ldl_personalize, 3);
    rb_define_method(cExtMAC, "tick", _ldl_tick, 0);
    rb_define_method(cExtMAC, "addChannel", _ldl_addChannel, 2);
    rb_define_method(cExtMAC, "removeChannel", _ldl_removeChannel, 1);
    rb_define_method(cExtMAC, "maskChannel", _ldl_maskChannel, 1);
    rb_define_method(cExtMAC, "unmaskChannel", _ldl_unmaskChannel, 1);
    rb_define_method(cExtMAC, "setRateAndPower", _ldl_setRateAndPower, 2);
    rb_define_method(cExtMAC, "join", _ldl_join, -1);
    rb_define_method(cExtMAC, "send_unconfirmed", _ldl_send_unconfirmed, -1);
    rb_define_method(cExtMAC, "send_confirmed", _ldl_send_confirmed, -1);    
    rb_define_method(cExtMAC, "io_event", io_event, 2);    
    
    cEUI64 = rb_const_get(cLDL, rb_intern("EUI64"));
    cKey = rb_const_get(cLDL, rb_intern("Key"));
    cError = rb_const_get(cLDL, rb_intern("Error"));
    cRadio = rb_const_get(cLDL, rb_intern("Radio"));
}

uint32_t Radio_resetHardware(struct lora_radio *self)
{
    return NUM2UINT(rb_funcall((VALUE)self, rb_intern("resetHardware"), 0));
}

void Radio_sleep(struct lora_radio *self)
{    
    rb_funcall((VALUE)self, rb_intern("sleep"), 0);
}

bool Radio_transmit(struct lora_radio *self, const struct lora_radio_tx_setting *settings, const void *data, uint8_t len)
{   
    VALUE params = rb_hash_new();
    
    rb_hash_aset(params, ID2SYM(rb_intern("freq")), NUM2UINT(settings->freq)); 
    rb_hash_aset(params, ID2SYM(rb_intern("preamble")), NUM2UINT(settings->preamble)); 
    rb_hash_aset(params, ID2SYM(rb_intern("power")), NUM2INT(settings->power)); 
    
    rb_hash_aset(params, ID2SYM(rb_intern("bw")), bw_to_symbol(settings->bw)); 
    rb_hash_aset(params, ID2SYM(rb_intern("sf")), sf_to_symbol(settings->sf)); 
    rb_hash_aset(params, ID2SYM(rb_intern("cr")), cr_to_symbol(settings->cr)); 
     
    return rb_funcall((VALUE)self, rb_intern("transmit"), 2, rb_str_new(data, len), params) == Qtrue;
}

bool Radio_receive(struct lora_radio *self, const struct lora_radio_rx_setting *settings)
{   
    VALUE params = rb_hash_new();
    
    rb_hash_aset(params, ID2SYM(rb_intern("freq")), NUM2UINT(settings->freq)); 
    rb_hash_aset(params, ID2SYM(rb_intern("preamble")), NUM2UINT(settings->preamble)); 
    rb_hash_aset(params, ID2SYM(rb_intern("timeout")), NUM2UINT(settings->timeout)); 
    
    rb_hash_aset(params, ID2SYM(rb_intern("bw")), bw_to_symbol(settings->bw)); 
    rb_hash_aset(params, ID2SYM(rb_intern("sf")), sf_to_symbol(settings->sf)); 
    rb_hash_aset(params, ID2SYM(rb_intern("cr")), cr_to_symbol(settings->cr)); 
    
    return rb_funcall((VALUE)self, rb_intern("receive"), 1, params) == Qtrue;
}

uint8_t Radio_collect(struct lora_radio *self, void *data, uint8_t max)
{    
    VALUE result = rb_funcall((VALUE)self, rb_intern("collect"), 0);
    
    uint8_t read = (RSTRING_LEN(result) > max) ? max : (uint8_t)RSTRING_LEN(result);
    
    (void)memcpy(data, RSTRING_PTR(result), read);
    
    return read;
}

void Radio_setEventHandler(struct lora_radio *self, void *receiver, radioEventCB cb)
{    
    rb_funcall((VALUE)self, rb_intern("set_mac"), 1, (VALUE)receiver);
}

static VALUE _ldl_alloc(VALUE klass)
{
    return Data_Wrap_Struct(klass, 0, free, calloc(1, sizeof(struct ldl)));
}

/* Create a new LDL instance
 * 
 * Regions supported:
 * 
 * - :eu_863_870
 * 
 * 
 * @param region [Symbol] optional region code
 * @param options [Hash] parameter hash
 * 
 * @option options [EUI64] :appEUI application identifier (defaults to 00-00-00-00-00-00-00-00)
 * @option options [EUI64] :devEUI device identifier (defaults to 00-00-00-00-00-00-00-00)
 * @option options [EUI64] :appKey application key (defaults to null key)
 * 
 * */
static VALUE _ldl_initialize(int argc, VALUE *argv, VALUE self)
{
    struct region_symbol_map {
        VALUE symbol;
        enum lora_region_id region;
    };
    
    struct region_symbol_map map[] = {
        {
            .symbol = ID2SYM(rb_intern("eu_863_870")),
            .region = EU_863_870
        }        
    };
    
    struct ldl *this;    
    enum lora_region_id region_id;
    size_t i;
    
    VALUE region;
    VALUE radio;
    VALUE options;
    VALUE appEUI;
    VALUE devEUI;
    VALUE appKey;
    
    Data_Get_Struct(self, struct ldl, this);
    
    (void)rb_scan_args(argc, argv, "10:", &radio, &options);
    
    if(region == Qnil){
        
        region = ID2SYM(rb_intern("eu_863_870"));
    }
    
    if(options == Qnil){
        
        options = rb_hash_new();
    }
    
    appEUI = rb_hash_aref(options, ID2SYM(rb_intern("appEUI")));
    devEUI = rb_hash_aref(options, ID2SYM(rb_intern("devEUI")));
    appKey = rb_hash_aref(options, ID2SYM(rb_intern("appKey")));
    region = rb_hash_aref(options, ID2SYM(rb_intern("region")));
    
    if(rb_obj_is_kind_of(radio, cRadio) != Qnil){
        
        rb_raise(rb_eTypeError, "radio must be kind of Radio");
    }
    
    if(region != Qnil){
        
        if(rb_obj_is_kind_of(region, rb_cSymbol) != Qtrue){
            
            rb_raise(rb_eTypeError, "region must be kind of symbol");
        }
        
        for(i=0U; i < sizeof(map)/sizeof(*map); i++){
            
            if(map[i].symbol == region){
                
                region = map[i].symbol;
                break;
            }
        }
        
        if(i == sizeof(map)/sizeof(*map)){
            
            rb_raise(rb_eArgError, "invalid region");
        }
    }
    else{
    
        region = ID2SYM(rb_intern("eu_863_870"));
    }
    
    if(!ldl_init(this, region_id, (struct lora_radio *)radio)){
        
        rb_raise(cError, "ldl_init() failed");
    }
    
    ldl_setResponseHandler(this, (void *)self, _response);
    
    rb_iv_set(self, "@radio", radio);
    
    if(appEUI != Qnil){
        
        if(rb_obj_is_kind_of(appEUI, cEUI64) != Qtrue){
            
            rb_raise(rb_eTypeError, "appEUI must be kind of EUI64");
        }
    }
    else{
    
        appEUI = rb_funcall(cEUI64, rb_intern("new"), 1, rb_str_new((char *)default_eui, sizeof(default_eui)-1));
    }
    
    if(devEUI != Qnil){
        
        if(rb_obj_is_kind_of(devEUI, cEUI64) != Qtrue){
            
            rb_raise(rb_eTypeError, "devEUI must be kind of EUI64");
        }
    }
    else{
    
        devEUI = rb_funcall(cEUI64, rb_intern("new"), 1, rb_str_new((char *)default_eui, sizeof(default_eui)-1));
    }
    
    if(appKey != Qnil){
        
        if(rb_obj_is_kind_of(appKey, cKey) != Qtrue){
            
            rb_raise(rb_eTypeError, "appKey must be kind of Key");
        }
    }
    else{
    
        appKey = rb_funcall(cKey, rb_intern("new"), 1, rb_str_new((char *)default_key, sizeof(default_key)-1));
    }
    
    rb_iv_set(self, "@appEUI", appEUI);
    rb_iv_set(self, "@devEUI", devEUI);
    rb_iv_set(self, "@appKey", appKey);
    
    rb_iv_set(self, "@tx_handler", Qnil);
    rb_iv_set(self, "@rx_handler", Qnil);
    rb_iv_set(self, "@join_handler", Qnil);
    
    rb_iv_set(self, "@rx_queue", rb_funcall(rb_const_get(rb_cObject, rb_intern("Queue")), rb_intern("new"), 0));
    
    return self;
}

static VALUE _ldl_initialize_copy(VALUE copy, VALUE orig)
{
    struct ldl *orig_ldl;
    struct ldl *copy_ldl;
    
    if(copy == orig){
        return copy;
    }
    
    if(TYPE(orig) != T_DATA){
        
        rb_raise(rb_eTypeError, "wrong argument type");
    }
    
    Data_Get_Struct(orig, struct ldl, orig_ldl);
    Data_Get_Struct(copy, struct ldl, copy_ldl);
    
    (void)memcpy(copy_ldl, orig_ldl, sizeof(struct ldl));
    
    return copy;
}

static VALUE _ldl_personalize(VALUE self, VALUE devAddr, VALUE nwkSKey, VALUE appSKey)
{
    struct ldl *this;    
    Data_Get_Struct(self, struct ldl, this);
    
    if(!ldl_personalize(this, (uint32_t)NUM2UINT(devAddr), RSTRING_PTR(rb_funcall(nwkSKey, rb_intern("value"), 0)), RSTRING_PTR(rb_funcall(appSKey, rb_intern("value"), 0)))){
        
        rb_raise(cError, "ldl_personalize() failed");
    }
    
    return self;
}

static VALUE _ldl_addChannel(VALUE self, VALUE freq, VALUE chIndex)
{
    struct ldl *this;    
    Data_Get_Struct(self, struct ldl, this);
    
    if(!ldl_addChannel(this, (uint32_t)NUM2UINT(freq), (uint8_t)NUM2UINT(chIndex))){
        
        rb_raise(cError, "ldl_addChannel() failed");    
    }
    
    return self;
}

static VALUE _ldl_removeChannel(VALUE self, VALUE chIndex)
{
    struct ldl *this;    
    Data_Get_Struct(self, struct ldl, this);
    
    ldl_removeChannel(this, (uint8_t)NUM2UINT(chIndex));
    
    return self;
}

static VALUE _ldl_maskChannel(VALUE self, VALUE chIndex)
{
    struct ldl *this;    
    Data_Get_Struct(self, struct ldl, this);
    
    if(!ldl_maskChannel(this, (uint8_t)NUM2UINT(chIndex))){
        
        rb_raise(cError, "ldl_maskChannel() failed");
    }
    
    return self;
}

static VALUE _ldl_unmaskChannel(VALUE self, VALUE chIndex)
{
    struct ldl *this;    
    Data_Get_Struct(self, struct ldl, this);
    
    ldl_unmaskChannel(this, (uint8_t)NUM2UINT(chIndex));
    
    return self;
}

static VALUE _ldl_setRateAndPower(VALUE self, VALUE rate, VALUE power)
{
    struct ldl *this;    
    Data_Get_Struct(self, struct ldl, this);
    
    if(!ldl_setRateAndPower(this, (uint8_t)NUM2UINT(rate), (uint8_t)NUM2UINT(power))){
        
        rb_raise(cError, "ldl_setRateAndPower() failed");
    }
    
    return self;
}

// want to pass a block for the callback
static VALUE _ldl_join(int argc, VALUE *argv, VALUE self)
{
    struct ldl *this;    
    Data_Get_Struct(self, struct ldl, this);    
    
    VALUE handler;  
    
    (void)rb_scan_args(argc, argv, "00&", &handler);
    
    if(!ldl_join(this)){
        
        rb_raise(cError, "ldl_join() failed");
    }
    
    rb_iv_set(self, "@join_handler", handler);
    
    return self;
}

static VALUE _ldl_send_unconfirmed(int argc, VALUE *argv, VALUE self)
{
    struct ldl *this;    
    Data_Get_Struct(self, struct ldl, this);
    
    VALUE port;
    VALUE data;
    VALUE handler;
    
    (void)rb_scan_args(argc, argv, "20&", &port, &data, &handler);
    
    if(!ldl_send(this, false, NUM2UINT(port), RSTRING_PTR(data), RSTRING_LEN(data))){
        
        rb_raise(cError, "ldl_send() failed");
    }
    
    rb_iv_set(self, "@tx_handler", handler);
    
    return self;
}

static VALUE _ldl_send_confirmed(int argc, VALUE *argv, VALUE self)
{
    struct ldl *this;    
    Data_Get_Struct(self, struct ldl, this);
  
    VALUE port;
    VALUE data;
    VALUE handler;
    
    (void)rb_scan_args(argc, argv, "20&", &port, &data, &handler);
    
    if(ldl_send(this, true, NUM2UINT(port), RSTRING_PTR(data), RSTRING_LEN(data))){
    
        rb_raise(cError, "ldl_send() failed");
    }
    
    rb_iv_set(self, "@tx_handler", handler);
    
    return self;
}

static VALUE _ldl_tick(VALUE self)
{
    struct ldl *this;    
    Data_Get_Struct(self, struct ldl, this);
    
    ldl_tick(this);
    
    return self;
}

static void _response(void *receiver, enum lora_mac_response_type type, const union lora_mac_response_arg *arg)
{
    VALUE self = (VALUE)receiver;
    
    VALUE handler;
    
    VALUE type_to_sym[] = {
        ID2SYM(rb_intern("tx_complete")),
        ID2SYM(rb_intern("tx_confirmed")),
        ID2SYM(rb_intern("tx_timeout")),
        ID2SYM(rb_intern("rx")),
        ID2SYM(rb_intern("join_success")),
        ID2SYM(rb_intern("join_timeout"))
    };

    switch(type){
    default:
    case LORA_MAC_TX_COMPLETE:
    case LORA_MAC_TX_CONFIRMED:
    case LORA_MAC_TX_TIMEOUT:
        handler = rb_iv_get(self, "@tx_handler");                        
        rb_funcall(handler, rb_intern("call"), 1, type_to_sym[type]);
        break;
    case LORA_MAC_RX:
    {
        VALUE rx_queue = rb_iv_get(self, "@rx_queue");
        VALUE hash = rb_hash_new();
        rb_hash_aset(hash, ID2SYM(rb_intern("port")), UINT2NUM(arg->rx.port));
        rb_hash_aset(hash, ID2SYM(rb_intern("msg")), rb_str_new((const char *)arg->rx.data, arg->rx.len));
        
        rb_funcall(rx_queue, rb_intern("push"), 1, hash);
    
        handler = rb_iv_get(self, "@rx_handler");    
        
        if(handler != Qnil){
        
            rb_funcall(handler, rb_intern("call"), 0);
        }
    }
        break;
    case LORA_MAC_JOIN_SUCCESS:
    case LORA_MAC_JOIN_TIMEOUT:
        handler = rb_iv_get(self, "@join_handler");    
        rb_funcall(handler, rb_intern("call"), 1, type_to_sym[type]);
        break;
    }   
}

static VALUE bw_to_symbol(enum lora_signal_bandwidth bw)
{
    VALUE map[] = {
        ID2SYM(rb_intern("bw_125")),
        ID2SYM(rb_intern("bw_250")),
        ID2SYM(rb_intern("bw_500")),
        ID2SYM(rb_intern("bw_fsk")),
    };
    
    return map[bw];
}

static VALUE sf_to_symbol(enum lora_spreading_factor sf)
{
    VALUE map[] = {
        ID2SYM(rb_intern("sf_7")),
        ID2SYM(rb_intern("sf_8")),
        ID2SYM(rb_intern("sf_9")),
        ID2SYM(rb_intern("sf_10")),
        ID2SYM(rb_intern("sf_11")),
        ID2SYM(rb_intern("sf_12")),
        ID2SYM(rb_intern("sf_fsk")),
    };
    
    return map[sf];
}

static VALUE cr_to_symbol(enum lora_coding_rate cr)
{
    VALUE map[] = {
        ID2SYM(rb_intern("cr_5")),
        ID2SYM(rb_intern("cr_6")),
        ID2SYM(rb_intern("cr_7")),
        ID2SYM(rb_intern("cr_8"))
    };
    
    return map[cr];    
}

static VALUE io_event(VALUE self, VALUE event, VALUE time)
{
    size_t i;
    struct ldl *this;    
    Data_Get_Struct(self, struct ldl, this);
    
    struct {
        
        enum lora_radio_event event;
        VALUE symbol;
    }
    map[] = {
        {
            .event = LORA_RADIO_TX_COMPLETE,
            .symbol = ID2SYM(rb_intern("tx_complete"))
        },
        {
            .event = LORA_RADIO_RX_READY,
            .symbol = ID2SYM(rb_intern("rx_ready"))
        },
        {
            .event = LORA_RADIO_RX_TIMEOUT,
            .symbol = ID2SYM(rb_intern("rx_timeout"))
        }
    };
    
    for(i=0U; i < sizeof(map)/sizeof(*map); i++){
        
        if(map[i].symbol == event){
    
            MAC_radioEvent(this, map[i].event, NUM2ULL(time));
            break;
        }
    }
    
    return self;
}
