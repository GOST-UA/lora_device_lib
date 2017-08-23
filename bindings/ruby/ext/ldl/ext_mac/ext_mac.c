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

static VALUE cLDL;
static VALUE cMAC;

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
static VALUE _ldl_interrupt(VALUE self, VALUE n, VALUE time);

static void _response(void *receiver, enum lora_mac_response_type type, const union lora_mac_response_arg *arg);

static VALUE radio_rx(VALUE self, VALUE message);

uint64_t System_getTime(void)
{
    return NUM2ULL(rb_funcall(rb_const_get(cLDL, rb_intern("SystemTime")), rb_intern("time"), 0));
}

void System_usleep(uint32_t interval)
{
}

void Init_ext_lora_device_lib(void)
{
    cLDL = rb_define_module("LoraDeviceLib");
    
    cLDL = rb_define_class_under(cLDL, "LDL", rb_cObject);
    rb_define_alloc_func(cLDL, _ldl_alloc);
    
    rb_define_method(cMAC, "initialize", _ldl_initialize, 1);
    rb_define_method(cMAC, "initialize_copy", _ldl_initialize_copy, 1);
    
    rb_define_method(cMAC, "personalize", _ldl_personalize, 3);
    rb_define_method(cMAC, "tick", _ldl_tick, 0);
    rb_define_method(cMAC, "addChannel", _ldl_addChannel, 2);
    rb_define_method(cMAC, "removeChannel", _ldl_removeChannel, 1);
    rb_define_method(cMAC, "maskChannel", _ldl_maskChannel, 1);
    rb_define_method(cMAC, "unmaskChannel", _ldl_unmaskChannel, 1);
    rb_define_method(cMAC, "setRateAndPower", _ldl_setRateAndPower, 2);
    rb_define_method(cMAC, "join", _ldl_join, -1);
    rb_define_method(cMAC, "send_unconfirmed", _ldl_send_unconfirmed, -1);
    rb_define_method(cMAC, "send_confirmed", _ldl_send_confirmed, -1);    
    
    rb_define_method(cLDL, "radio_rx", radio_rx, 1);    
}

void Radio_init(struct lora_radio *self, const struct lora_board *board)
{    
    rb_iv_set(self, "@radio", );
}

uint32_t Radio_resetHardware(struct lora_radio *self)
{
    
    
    return 0U;
}

void Radio_sleep(struct lora_radio *self)
{    
    rb_funcall(rb_iv_get(self, "@radio"), rb_intern("receive")) == Qtrue);
}

bool Radio_transmit(struct lora_radio *self, const struct lora_radio_setting *settings, const void *data, uint8_t len)
{    
    return(rb_funcall(rb_iv_get(self, "@radio"), rb_intern("transmit")) == Qtrue);
}

bool Radio_receive(struct lora_radio *self, bool continuous, const struct lora_radio_setting *settings)
{    
    return(rb_funcall(rb_iv_get(self, "@radio"), rb_intern("receive")) == Qtrue);
}

uint8_t Radio_collect(struct lora_radio *self, void *data, uint8_t max)
{    
    VALUE buffer = rb_iv_get(self, "@rx_buffer");
    
    uint8_t read = (RSTRING_LEN(buffer) > max) ? max : (uint8_t)RSTRING_LEN(buffer);
    
    (void)memcpy(data, RSTRING_PTR(buffer), read);
    
    return read;
}

void Radio_interrupt(struct lora_radio *self, uint8_t n, uint64_t time)
{   
     
}

void Radio_setEventHandler(struct lora_radio *self, void *receiver, radioEventCB cb)
{
    
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
 * @param region [Symbol]
 * 
 * */
static VALUE _ldl_initialize(VALUE self, VALUE region)
{
    struct ldl *this;    
    enum lora_region_id region_id;
    size_t i;
    
    Data_Get_Struct(self, struct ldl, this);
    
    ldl_setResponseHandler(this, (void *)self, _response);  
    
    struct region_symbol_map {
        VALUE symbol;
        enum lora_region_id region;
    };
    
    struct radio_symbol_map {
        VALUE symbol;
        enum lora_board board;        
    };
    
    struct region_symbol_map map[] = {
        {
            .symbol = ID2SYM(rb_intern("eu_863_870")),
            .region_id = EU_863_870
        }        
    };
    
    for(i=0U; i < sizeof(map)/sizeof(*map); i++){
        
        if(map[i].symbol == region){
            
            region_id = map[i].region_id;
            break;
        }
    }
    
    if(i == sizeof(map)/sizeof(*map)){
        
        rb_raise(rb_eTypeError, "invalid region")
    }
    
    if(!ldl_init(this, region_id, NULL)){
        
        rb_raise(rb_const_get(cLDL, rb_intern("LoraError")), "ldl_init() failed");
    }
    
    rb_iv_set(self, "@tx_handler", Qnil);
    rb_iv_set(self, "@rx_handler", Qnil);
    rb_iv_set(self, "@join_handler", Qnil);
    rb_iv_set(self, "@rx_queue", rb_funcall(rb_const_get(rb_cObject, rb_intern("Queue")), rb_intern("new"), 0));
    
    rb_iv_set(self, "@radio_state", ID2SYM(rb_intern("sleep")));
    
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
    
    if(!ldl_personalize(this, (uint32_t)NUM2UINT(devAddr), RSTRING_PTR(nwkSKey), RSTRING_PTR(appSKey))){
        
        rb_raise(rb_const_get(cLDL, rb_intern("LoraError")), "ldl_personalize() failed");
    }
    
    return self;
}

static VALUE _ldl_addChannel(VALUE self, VALUE freq, VALUE chIndex)
{
    struct ldl *this;    
    Data_Get_Struct(self, struct ldl, this);
    
    if(!ldl_addChannel(this, (uint32_t)NUM2UINT(freq), (uint8_t)NUM2UINT(chIndex))){
        
        rb_raise(rb_const_get(cLDL, rb_intern("LoraError")), "ldl_addChannel() failed");    
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
        
        rb_raise(rb_const_get(cLDL, rb_intern("LoraError")), "ldl_maskChannel() failed");
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
        
        rb_raise(rb_const_get(cLDL, rb_intern("LoraError")), "ldl_setRateAndPower() failed");
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
        
        rb_raise(rb_const_get(cLDL, rb_intern("LoraError")), "ldl_join() failed");
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
        
        rb_raise(rb_const_get(cLDL, rb_intern("LoraError")), "ldl_send() failed");
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
    
        rb_raise(rb_const_get(cLDL, rb_intern("LoraError")), "ldl_send() failed");
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

static VALUE _ldl_interrupt(VALUE self, VALUE n, VALUE time)
{
    struct ldl *this;    
    Data_Get_Struct(self, struct ldl, this);
    
    idl_interrupt(this, NUM2UINT(n), NUM2UINT(time));    
    
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

static VALUE radio_rx(VALUE self, VALUE message)
{
    // check if radio is waiting to rx
}
