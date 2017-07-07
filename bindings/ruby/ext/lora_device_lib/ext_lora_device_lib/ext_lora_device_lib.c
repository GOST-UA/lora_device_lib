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
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>

#include "lora_device_lib.h"

static VALUE cLDL;
static VALUE cLoraDeviceLib;

static VALUE ldl_initialize(VALUE self, VALUE radio);
static VALUE ldl_initialize_copy(VALUE copy, VALUE orig);
static VALUE ldl_alloc(VALUE klass);

static VALUE _ldl_personalize(VALUE self, VALUE devAddr, VALUE nwkSKey, VALUE appSKey);
static VALUE _ldl_tick(VALUE self);
static VALUE _ldl_addChannel(VALUE self, VALUE freq, VALUE chIndex);
static VALUE _ldl_removeChannel(VALUE self, VALUE chIndex);
static VALUE _ldl_maskChannel(VALUE self, VALUE chIndex);
static VALUE _ldl_unmaskChannel(VALUE self, VALUE chIndex);
static VALUE _ldl_setRateAndPower(VALUE self, VALUE rate, VALUE power);
static VALUE _ldl_join(VALUE self);
static VALUE _ldl_send_unconfirmed(int argc, VALUE *argv, VALUE self);
static VALUE _ldl_send_confirmed(int argc, VALUE *argv, VALUE self);
static VALUE _ldl_interrupt(VALUE self, VALUE n, VALUE time);

static void _txComplete(void *receiver, enum lora_tx_status status);
static void _rxComplete(void *receiver, uint8_t port, const void *data, uint8_t len);
static void _joinComplete(void *receiver, bool noResponse);

static void board_select(void *receiver, bool state);
static void board_reset(void *receiver, bool state);
static void board_reset_wait(void *receiver);
static void board_write(void *receiver, uint8_t data);
static uint8_t board_read(void *receiver);

void Init_ext_lora_device_lib(void)
{
    cLoraDeviceLib = rb_define_module("LoraDeviceLib");
    cLDL = rb_define_class_under(cLoraDeviceLib, "LDL", rb_cObject);
    rb_define_alloc_func(cLDL, ldl_alloc);
    
    rb_define_method(cLDL, "initialize", ldl_initialize, 1);
    rb_define_method(cLDL, "initialize_copy", ldl_initialize_copy, 1);
    
    rb_define_method(cLDL, "personalize", _ldl_personalize, 3);
    rb_define_method(cLDL, "tick", _ldl_tick, 0);
    rb_define_method(cLDL, "addChannel", _ldl_addChannel, 2);
    rb_define_method(cLDL, "removeChannel", _ldl_removeChannel, 1);
    rb_define_method(cLDL, "maskChannel", _ldl_maskChannel, 1);
    rb_define_method(cLDL, "unmaskChannel", _ldl_unmaskChannel, 1);
    rb_define_method(cLDL, "setRateAndPower", _ldl_setRateAndPower, 2);
    rb_define_method(cLDL, "join", _ldl_join, -1);
    rb_define_method(cLDL, "send_unconfirmed", _ldl_send_unconfirmed, -1);
    rb_define_method(cLDL, "send_confirmed", _ldl_send_confirmed, -1);
}


static VALUE _ldl_initialize(int argc, VALUE *argv, VALUE self)
{
    struct ldl *this;    
    Data_Get_Struct(self, struct ldl, this);
    
    VALUE port;
    VALUE data;
    VALUE handler;
    
    (void)rb_scan_args(argc, argv, "20&", &port, &data, &handler);
}

static VALUE ldl_initialize(VALUE self, VALUE board)
{
    struct ldl *this;
    
    Data_Get_Struct(self, struct ldl, this);
    
    ldl_setReceiveHandler(this, (void *)self, _rxComplete);  
    ldl_setTransmitHandler(this, (void *)self, _txComplete);  
    ldl_setJoinHandler(this, (void *)self, _joinComplete);  
    
    struct lora_board boardAdapter = {
        .receiver = (void *)board,
        .select = board_select,
        .reset = board_reset,
        .reset_wait = board_reset_wait,
        .write = board_write,
        .read = board_read
    };
    
    *this = ldl_new(EU_863_870, LORA_RADIO_SX1272, &boardAdapter);
    
    rb_iv_set(self, "@board", board);
    rb_iv_set(self, "@tx_handler", Qnil);
    rb_iv_set(self, "@rx_handler", Qnil);
    rb_iv_set(self, "@join_handler", Qnil);
    
    // instance of a queue
    //rb_iv_set(self, "@rx_queue", );
    
    return self;
}

static VALUE ldl_initialize_copy(VALUE copy, VALUE orig)
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

static VALUE ldl_alloc(VALUE klass)
{
    return Data_Wrap_Struct(klass, NULL, free, calloc(1, sizeof(struct ldl)));
}

static VALUE _ldl_personalize(VALUE self, VALUE devAddr, VALUE nwkSKey, VALUE appSKey)
{
    struct ldl *this;    
    Data_Get_Struct(self, struct ldl, this);
    
    if(!ldl_personalize(this, (uint32_t)NUM2UINT(devAddr), RSTRING_PTR(nwkSKey), RSTRING_PTR(appSKey))){
        
        rb_raise(rb_eException, "personalise failed");
    }
    
    return self;
}

static VALUE _ldl_addChannel(VALUE self, VALUE freq, VALUE chIndex)
{
    struct ldl *this;    
    Data_Get_Struct(self, struct ldl, this);
    
    if(!ldl_addChannel(this, (uint32_t)NUM2UINT(freq), (uint8_t)NUM2UINT(chIndex))){
        
        rb_raise(rb_eException, "addChannel failed");
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
        
        rb_raise(rb_eException, "maskChannel failed");
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
}

// want to pass a block for the callback
static VALUE _ldl_join(VALUE self)
{
    struct ldl *this;    
    Data_Get_Struct(self, struct ldl, this);    
    
    if(!ldl_join(this)){
        
        rb_raise(rb_eException, "ldl_send()");
    }
    
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
        
        rb_raise(rb_eException, "ldl_send()");
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
    
        rb_raise(rb_eException, "ldl_send()");
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
}

static void _txComplete(void *receiver, enum lora_tx_status status)
{
    VALUE self = (VALUE)receiver;
    
    VALUE handler = rb_iv_get(self, "@tx_handler");    
    VALUE status_sym;
    
    VALUE status_to_sym[] = {
        ID2SYM(rb_intern("tx_complete")),
        ID2SYM(rb_intern("tx_confirmed")),
        ID2SYM(rb_intern("tx_timeout"))
    };
    
    rb_funcall(handler, rb_intern("call"), 1, status_to_sym[status]);
}

static void _rxComplete(void *receiver, uint8_t port, const void *data, uint8_t len)
{
    VALUE self = (VALUE)receiver;    
}

static void _joinComplete(void *receiver, bool noResponse)
{
    VALUE self = (VALUE)receiver;    
}

static void board_select(void *receiver, bool state)
{
    VALUE self = (VALUE)receiver;    
    (void)rb_funcall(self, rb_intern("select"), 1, (state) ? Qtrue : Qfalse);
}

static void board_reset(void *receiver, bool state)
{
    VALUE self = (VALUE)receiver;
    (void)rb_funcall(self, rb_intern("reset"), 1, (state) ? Qtrue : Qfalse);
}

static void board_reset_wait(void *receiver)
{
    VALUE self = (VALUE)receiver;
    (void)rb_funcall(self, rb_intern("reset_wait"), 0);
}

static void board_write(void *receiver, uint8_t data)
{
    VALUE self = (VALUE)receiver;
    (void)rb_funcall(self, rb_intern("write"), 1, UINT2NUM(data));
}

static uint8_t board_read(void *receiver)
{
    VALUE self = (VALUE)receiver;
    return (uint8_t)NUM2UINT(rb_funcall(self, rb_intern("read"), 0));
}
