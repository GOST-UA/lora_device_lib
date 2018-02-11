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

#include "lora_mac.h"

static const uint8_t default_key[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
static const uint8_t default_eui[] = "\x00\x00\x00\x00\x00\x00\x00\x00";

static VALUE cLDL;
static VALUE cExtMAC;
static VALUE cEUI64;
static VALUE cKey;
static VALUE cError;
static VALUE cRadio;

static VALUE initialize(int argc, VALUE *argv, VALUE self);
static VALUE initialize_copy(VALUE copy, VALUE orig);
static VALUE alloc_state(VALUE klass);

static VALUE tick(VALUE self);
static VALUE setRate(VALUE self, VALUE rate);
static VALUE setPower(VALUE self, VALUE power);
static VALUE join(int argc, VALUE *argv, VALUE self);
static VALUE data(int argc, VALUE *argv, VALUE self);
static VALUE io_event(VALUE self, VALUE event, VALUE time);
static VALUE ticksUntilNextChannel(VALUE self);
static VALUE ticksUntilNextEvent(VALUE self);
static VALUE transmitTimeUp(VALUE self, VALUE bw, VALUE sf, VALUE size);
static VALUE transmitTimeDown(VALUE self, VALUE bw, VALUE sf, VALUE size);

static void response(void *receiver, enum lora_mac_response_type type, const union lora_mac_response_arg *arg);

static VALUE bw_to_number(enum lora_signal_bandwidth bw);
static VALUE sf_to_number(enum lora_spreading_factor sf);
static VALUE cr_to_number(enum lora_coding_rate cr);
static enum lora_spreading_factor number_to_sf(VALUE sf);
static enum lora_signal_bandwidth number_to_bw(VALUE bw);



/* functions **********************************************************/

uint64_t System_time(void)
{
    return NUM2ULL(rb_funcall(rb_const_get(cLDL, rb_intern("SystemTime")), rb_intern("time"), 0));
}

void System_sleep(uint32_t interval)
{
    //don't block the thread because we are inside a mutex
    //(void)rb_funcall(rb_const_get(cLDL, rb_intern("SystemTime")), rb_intern("wait"), 1, UINT2NUM(interval));
}

uint8_t System_rand(void)
{
    return (uint8_t)rb_genrand_int32();
}

void System_atomic_setPtr(void **receiver, void *value)
{
    *receiver = value;  //fixme
}

void System_getAppEUI(void *receiver, void *eui)
{
    VALUE self = (VALUE)receiver;
    
    VALUE appEUI = rb_iv_get(self, "@appEUI");
    
    (void)memcpy(eui, RSTRING_PTR(rb_funcall(appEUI, rb_intern("bytes"), 0)), sizeof(default_eui));
}

void System_getDevEUI(void *receiver, void *eui)
{
    VALUE self = (VALUE)receiver;    
    VALUE devEUI = rb_iv_get(self, "@devEUI");
    
    (void)memcpy(eui, RSTRING_PTR(rb_funcall(devEUI, rb_intern("bytes"), 0)), sizeof(default_eui));
}

void System_getAppKey(void *receiver, void *key)
{
    VALUE self = (VALUE)receiver;    
    VALUE appKey = rb_iv_get(self, "@appKey");
    
    (void)memcpy(key, RSTRING_PTR(rb_funcall(appKey, rb_intern("value"), 0)), sizeof(default_key));
}

void System_getNwkSKey(void *receiver, void *key)
{
    VALUE k = rb_iv_get((VALUE)receiver, "@nwkSKey");
 
    (void)memcpy(key, RSTRING_PTR(k), RSTRING_LEN(k));    
}

void System_getAppSKey(void *receiver, void *key)
{
    VALUE k = rb_iv_get((VALUE)receiver, "@appSKey");
 
    (void)memcpy(key, RSTRING_PTR(k), RSTRING_LEN(k));
}

void System_setNwkSKey(void *receiver, const void *key)
{
    rb_iv_set((VALUE)receiver, "@nwkSKey", rb_str_new((const char *)key, 16U));
}

void System_setAppSKey(void *receiver, const void *key)
{
    rb_iv_set((VALUE)receiver, "@appSKey", rb_str_new((const char *)key, 16U));
}

void System_setDevAddr(void *receiver, uint32_t devAddr)
{
    rb_iv_set((VALUE)receiver, "@devAddr", UINT2NUM(devAddr));
}

uint32_t System_getDevAddr(void *receiver)
{
    return NUM2UINT(rb_iv_get((VALUE)receiver, "@devAddr"));
}

bool System_getChannel(void *receiver, uint8_t chIndex, uint32_t *freq, uint8_t *minRate, uint8_t *maxRate)
{
    bool retval = false;
    
    VALUE channels = rb_iv_get((VALUE)receiver, "@channels");    
    VALUE channel = rb_ary_entry(channels, UINT2NUM(chIndex));
    
    if(channel != Qnil){
        
        *freq = NUM2UINT(rb_hash_aref(channel, ID2SYM(rb_intern("freq"))));
        *minRate = NUM2UINT(rb_hash_aref(channel, ID2SYM(rb_intern("minRate"))));
        *maxRate = NUM2UINT(rb_hash_aref(channel, ID2SYM(rb_intern("maxRate"))));
        
        retval = true;
    }
    
    return retval;    
}

bool System_setChannel(void *receiver, uint8_t chIndex, uint32_t freq, uint8_t minRate, uint8_t maxRate)
{
    bool retval = false;
    
    VALUE channels = rb_iv_get((VALUE)receiver, "@channels");    
    VALUE channel = rb_ary_entry(channels, UINT2NUM(chIndex));
    
    if(channel != Qnil){
        
        rb_hash_aset(channel, ID2SYM(rb_intern("freq")), UINT2NUM(freq));
        rb_hash_aset(channel, ID2SYM(rb_intern("minRate")), UINT2NUM(minRate));
        rb_hash_aset(channel, ID2SYM(rb_intern("maxRate")), UINT2NUM(maxRate));
        
        rb_ary_store(channels, UINT2NUM(chIndex), channel);
        
        retval = true;
    }
    
    return retval;    
}

bool System_maskChannel(void *receiver, uint8_t chIndex)
{
    bool retval = false;
    
    VALUE channels = rb_iv_get((VALUE)receiver, "@channels");    
    VALUE channel = rb_ary_entry(channels, UINT2NUM(chIndex));
    
    if(channel != Qnil){
    
        rb_hash_aset(channel, ID2SYM(rb_intern("masked")), Qtrue);
        
        retval = true;
    }
    
    return retval;
}

bool System_unmaskChannel(void *receiver, uint8_t chIndex)
{
    bool retval = false;
    
    VALUE channels = rb_iv_get((VALUE)receiver, "@channels");    
    VALUE channel = rb_ary_entry(channels, UINT2NUM(chIndex));
    
    if(channel != Qnil){
    
        rb_hash_aset(channel, ID2SYM(rb_intern("masked")), Qfalse);
        
        retval = true;
    }
    
    return retval;
}

bool System_channelIsMasked(void *receiver, uint8_t chIndex)
{
    bool retval = false;
    
    VALUE channels = rb_iv_get((VALUE)receiver, "@channels");    
    VALUE channel = rb_ary_entry(channels, UINT2NUM(chIndex));
    
    if(channel != Qnil){
    
        retval = ( rb_hash_aref(channel, ID2SYM(rb_intern("masked"))) == Qtrue ) ? true : false;
    }
    
    return retval;
}

uint8_t System_getRX1DROffset(void *receiver)
{
    return NUM2UINT(rb_iv_get((VALUE)receiver, "@rx1DROffset"));
}

uint8_t System_getMaxDutyCycle(void *receiver)
{    
    return NUM2UINT(rb_iv_get((VALUE)receiver, "@maxDutyCycle"));
}

uint8_t System_getRX1Delay(void *receiver)
{
    return NUM2UINT(rb_iv_get((VALUE)receiver, "@rx1Delay"));
}

uint8_t System_getNbTrans(void *receiver)
{
    return NUM2UINT(rb_iv_get((VALUE)receiver, "@nbTrans"));
}

uint8_t System_getTXPower(void *receiver)
{
    return NUM2UINT(rb_iv_get((VALUE)receiver, "@txPower"));
}

uint8_t System_getTXRate(void *receiver)
{
    return NUM2UINT(rb_iv_get((VALUE)receiver, "@txRate"));
}

uint32_t System_getRX2Freq(void *receiver)
{
    return NUM2UINT(rb_iv_get((VALUE)receiver, "@rx2Freq"));
}

uint8_t System_getRX2DataRate(void *receiver)
{
    return NUM2UINT(rb_iv_get((VALUE)receiver, "@rx2DataRate"));
}

void System_setRX1DROffset(void *receiver, uint8_t value)
{
    rb_iv_set((VALUE)receiver, "@rx1DROffset", UINT2NUM(value));
}

void System_setMaxDutyCycle(void *receiver, uint8_t value)
{
    rb_iv_set((VALUE)receiver, "@maxDutyCycle", UINT2NUM(value));
}

void System_setRX1Delay(void *receiver, uint8_t value)
{
    rb_iv_set((VALUE)receiver, "@rx1Delay", UINT2NUM(value));
}

void System_setTXPower(void *receiver, uint8_t value)
{
    rb_iv_set((VALUE)receiver, "@txPower", UINT2NUM(value));
}

void System_setNbTrans(void *receiver, uint8_t value)
{
    rb_iv_set((VALUE)receiver, "@nbTrans", UINT2NUM(value));
}

void System_setTXRate(void *receiver, uint8_t value)
{
    rb_iv_set((VALUE)receiver, "@txRate", UINT2NUM(value));
}

void System_setRX2Freq(void *receiver, uint32_t value)
{
    rb_iv_set((VALUE)receiver, "@rx2Freq", UINT2NUM(value));
}

void System_setRX2DataRate(void *receiver, uint8_t value)
{
    rb_iv_set((VALUE)receiver, "@rx2DataRate", UINT2NUM(value));
}

uint16_t System_getUp(void *receiver)
{
    return NUM2UINT(rb_iv_get((VALUE)receiver, "@upCount"));
}

uint16_t System_incrementUp(void *receiver)
{
    uint16_t retval = NUM2UINT(rb_iv_get((VALUE)receiver, "@upCount"));
    
    rb_iv_set((VALUE)receiver, "@upCount", retval + 1U);
    
    return retval;    
}

void System_resetUp(void *receiver)
{
    rb_iv_set((VALUE)receiver, "@upCount", 0U);
}

uint16_t System_getDown(void *receiver)
{
    return NUM2UINT(rb_iv_get((VALUE)receiver, "@downCount"));
}

uint8_t System_getBatteryLevel(void *receiver)
{
    return 255U;
}

bool System_receiveDown(void *receiver, uint16_t counter, uint16_t maxGap)
{
    bool retval = false;
    uint16_t value = NUM2UINT(rb_iv_get((VALUE)receiver, "@upCount"));
    
    if((uint32_t)counter < ((uint32_t)value + (uint32_t)maxGap)){
        
        rb_iv_set((VALUE)receiver, "@downCount", UINT2NUM(counter));        
        retval = true;
    }
    
    return retval;
}

void System_resetDown(void *receiver)
{
    rb_iv_set((VALUE)receiver, "@downCount", 0U);
}

void Init_ext_mac(void)
{
    cLDL = rb_define_module("LDL");
    
    cExtMAC = rb_define_class_under(cLDL, "ExtMAC", rb_cObject);
    rb_define_alloc_func(cExtMAC, alloc_state);
    
    rb_define_method(cExtMAC, "initialize", initialize, -1);
    rb_define_method(cExtMAC, "initialize_copy", initialize_copy, 1);
        
    rb_define_method(cExtMAC, "tick", tick, 0);
    rb_define_method(cExtMAC, "rate=", setRate, 1);
    rb_define_method(cExtMAC, "power=", setPower, 1);
    rb_define_method(cExtMAC, "join", join, -1);
    rb_define_method(cExtMAC, "data", data, -1);
    rb_define_method(cExtMAC, "io_event", io_event, 2);    
    rb_define_method(cExtMAC, "ticksUntilNextEvent", ticksUntilNextEvent, 0);    
    rb_define_method(cExtMAC, "ticksUntilNextChannel", ticksUntilNextChannel, 0);    
    
    rb_define_singleton_method(cExtMAC, "transmitTimeUp", transmitTimeUp, 3);
    rb_define_singleton_method(cExtMAC, "transmitTimeDown", transmitTimeDown, 3);
    
    cEUI64 = rb_const_get(cLDL, rb_intern("EUI64"));
    cKey = rb_const_get(cLDL, rb_intern("Key"));
    cError = rb_const_get(cLDL, rb_intern("Error"));
    cRadio = rb_const_get(cLDL, rb_intern("Radio"));    
    
    rb_const_set(cExtMAC, rb_intern("TIMEBASE"), UINT2NUM(LORA_TIMEBASE));
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
    
    rb_hash_aset(params, ID2SYM(rb_intern("freq")), UINT2NUM(settings->freq)); 
    rb_hash_aset(params, ID2SYM(rb_intern("preamble")), UINT2NUM(settings->preamble)); 
    rb_hash_aset(params, ID2SYM(rb_intern("power")), UINT2NUM(settings->power)); 
    
    rb_hash_aset(params, ID2SYM(rb_intern("bw")), bw_to_number(settings->bw)); 
    rb_hash_aset(params, ID2SYM(rb_intern("sf")), sf_to_number(settings->sf)); 
    rb_hash_aset(params, ID2SYM(rb_intern("cr")), cr_to_number(settings->cr)); 
    
    rb_hash_aset(params, ID2SYM(rb_intern("channel")), UINT2NUM(settings->channel)); 
     
    return rb_funcall((VALUE)self, rb_intern("transmit"), 2, rb_str_new(data, len), params) == Qtrue;
}

bool Radio_receive(struct lora_radio *self, const struct lora_radio_rx_setting *settings)
{   
    VALUE params = rb_hash_new();
    
    rb_hash_aset(params, ID2SYM(rb_intern("freq")), UINT2NUM(settings->freq)); 
    rb_hash_aset(params, ID2SYM(rb_intern("preamble")), UINT2NUM(settings->preamble)); 
    rb_hash_aset(params, ID2SYM(rb_intern("timeout")), UINT2NUM(settings->timeout)); 
    
    rb_hash_aset(params, ID2SYM(rb_intern("bw")), bw_to_number(settings->bw)); 
    rb_hash_aset(params, ID2SYM(rb_intern("sf")), sf_to_number(settings->sf)); 
    rb_hash_aset(params, ID2SYM(rb_intern("cr")), cr_to_number(settings->cr)); 
    
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
}

static VALUE alloc_state(VALUE klass)
{
    return Data_Wrap_Struct(klass, 0, free, calloc(1, sizeof(struct lora_mac)));
}

static void initChannels(VALUE self, enum lora_region_id region)
{
    VALUE channels = rb_ary_new();
    uint8_t i;
    
    for(i=0U; i < Region_numChannels(Region_getRegion(region)); i++){
        
        VALUE channel = rb_hash_new();
        
        rb_hash_aset(channel, ID2SYM(rb_intern("chIndex")), UINT2NUM(i));
        rb_hash_aset(channel, ID2SYM(rb_intern("freq")), UINT2NUM(0));
        rb_hash_aset(channel, ID2SYM(rb_intern("minRate")), UINT2NUM(0));
        rb_hash_aset(channel, ID2SYM(rb_intern("maxRate")), UINT2NUM(0));
        rb_hash_aset(channel, ID2SYM(rb_intern("mask")), Qfalse);
        
        rb_ary_push(channels, channel);
    }
    
    rb_iv_set(self, "@channels", channels);
}

/* static functions ***************************************************/

/* Create a new LDL instance
 * 
 * Regions supported:
 * 
 * - :eu_863_870
 * 
 * 
 * @param radio [Radio]
 * @param options [Hash] parameter hash
 * 
 * @option options [EUI64] :region 
 * @option options [EUI64] :appEUI application identifier (defaults to 00-00-00-00-00-00-00-00)
 * @option options [EUI64] :devEUI device identifier (defaults to 00-00-00-00-00-00-00-00)
 * @option options [Key] :appKey application key (defaults to null key)
 * 
 * */
static VALUE initialize(int argc, VALUE *argv, VALUE self)
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
    
    struct lora_mac *this;    
    enum lora_region_id region_id;
    size_t i;
    
    VALUE region;
    VALUE radio;
    VALUE options;
    VALUE appEUI;
    VALUE devEUI;
    VALUE appKey;
    
    Data_Get_Struct(self, struct lora_mac, this);
    
    (void)rb_scan_args(argc, argv, "10:", &radio, &options);
    
    if(options == Qnil){
        
        options = rb_hash_new();
    }
    
    appEUI = rb_hash_aref(options, ID2SYM(rb_intern("appEUI")));
    devEUI = rb_hash_aref(options, ID2SYM(rb_intern("devEUI")));
    appKey = rb_hash_aref(options, ID2SYM(rb_intern("appKey")));
    region = rb_hash_aref(options, ID2SYM(rb_intern("region")));

    if(rb_obj_is_kind_of(radio, cRadio) != Qtrue){
        
        rb_raise(rb_eTypeError, "radio must be kind of Radio");
    }
    
    if(region != Qnil){
        
        if(rb_obj_is_kind_of(region, rb_cSymbol) != Qtrue){
            
            rb_raise(rb_eTypeError, "region must be kind of symbol");
        }
        
        for(i=0U; i < sizeof(map)/sizeof(*map); i++){
            
            if(map[i].symbol == region){
                
                region_id = map[i].region;
                break;
            }
        }
        
        if(i == sizeof(map)/sizeof(*map)){
            
            rb_raise(rb_eArgError, "invalid region");
        }
    }
    else{
    
        region = ID2SYM(rb_intern("eu_863_870"));
        region_id = EU_863_870;
    }

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
    
    VALUE appSKey = rb_funcall(cKey, rb_intern("new"), 1, rb_str_new((char *)default_key, sizeof(default_key)-1));
    VALUE nwkSKey = rb_funcall(cKey, rb_intern("new"), 1, rb_str_new((char *)default_key, sizeof(default_key)-1));
        
    rb_iv_set(self, "@appEUI", appEUI);
    rb_iv_set(self, "@devEUI", devEUI);
    rb_iv_set(self, "@appKey", appKey);
    rb_iv_set(self, "@appSKey", appSKey);
    rb_iv_set(self, "@nwkSKey", nwkSKey);
    rb_iv_set(self, "@devAddr", UINT2NUM(0U));
    
    rb_iv_set(self, "@tx_handle", Qnil);
    rb_iv_set(self, "@rx_handle", Qnil);
    
    initChannels(self, region_id);
    
    MAC_init(this, (void *)self, region_id, (struct lora_radio *)radio, (void *)self, response);

    MAC_restoreDefaults(this);
    
    return self;
}

static VALUE initialize_copy(VALUE copy, VALUE orig)
{
    struct lora_mac *orig_ldl;
    struct lora_mac *copy_ldl;
    
    if(copy == orig){
        return copy;
    }
    
    if(TYPE(orig) != T_DATA){
        
        rb_raise(rb_eTypeError, "wrong argument type");
    }
    
    Data_Get_Struct(orig, struct lora_mac, orig_ldl);
    Data_Get_Struct(copy, struct lora_mac, copy_ldl);
    
    (void)memcpy(copy_ldl, orig_ldl, sizeof(struct lora_mac));
    
    return copy;
}

static VALUE setRate(VALUE self, VALUE rate)
{
    struct lora_mac *this;    
    Data_Get_Struct(self, struct lora_mac, this);
    
    if(!MAC_setRate(this, (uint8_t)NUM2UINT(rate))){
        
        rb_raise(cError, "MAC_setRate() failed");
    }
    
    return self;
}

static VALUE setPower(VALUE self, VALUE power)
{
    struct lora_mac *this;    
    Data_Get_Struct(self, struct lora_mac, this);
    
    if(!MAC_setPower(this, (uint8_t)NUM2UINT(power))){
        
        rb_raise(cError, "MAC_setPower() failed");
    }
    
    return self;
}

// want to pass a block for the callback
static VALUE join(int argc, VALUE *argv, VALUE self)
{
    struct lora_mac *this;    
    Data_Get_Struct(self, struct lora_mac, this);    
    
    VALUE handler;  
    
    (void)rb_scan_args(argc, argv, "00&", &handler);
    
    if(!MAC_join(this)){
        
        rb_raise(cError, "MAC_join() failed");
    }
    
    rb_iv_set(self, "@tx_handle", handler);
    
    return self;
}

static VALUE data(int argc, VALUE *argv, VALUE self)
{
    struct lora_mac *this;    
    Data_Get_Struct(self, struct lora_mac, this);
    
    VALUE port;
    VALUE data;
    VALUE handler;
    VALUE opts;
    
    (void)rb_scan_args(argc, argv, "20:&", &port, &data, &handler, &opts);
    
    if(opts != Qnil){
        
        opts = rb_hash_new();
    }
    
    if(rb_hash_aref(opts, ID2SYM(rb_intern("confirmed"))) == Qnil){
        
        if(!MAC_send(this, false, NUM2UINT(port), RSTRING_PTR(data), RSTRING_LEN(data))){
            
            rb_raise(cError, "MAC_send() failed");
        }            
    }
    else{
        
        if(!MAC_send(this, true, NUM2UINT(port), RSTRING_PTR(data), RSTRING_LEN(data))){
            
            rb_raise(cError, "MAC_send() failed");
        }            
    }   
    
    rb_iv_set(self, "@tx_handle", handler);
    
    return self;
}

static VALUE tick(VALUE self)
{
    struct lora_mac *this;    
    Data_Get_Struct(self, struct lora_mac, this);
    
    MAC_tick(this);
    
    return self;
}

static void response(void *receiver, enum lora_mac_response_type type, const union lora_mac_response_arg *arg)
{
    VALUE self = (VALUE)receiver;
    
    VALUE handler;
    
    VALUE type_to_sym[] = {
        ID2SYM(rb_intern("ready")),
        ID2SYM(rb_intern("timeout")),
        ID2SYM(rb_intern("rx"))        
    };

    VALUE tx_handle = rb_iv_get(self, "@tx_handle");        
    VALUE rx_handle = rb_iv_get(self, "@rx_handle");        

    switch(type){
    default:
    case LORA_MAC_READY:
    case LORA_MAC_TIMEOUT:
    
        if(tx_handle != Qnil){                        
            
            rb_funcall(tx_handle, rb_intern("call"), 1, type_to_sym[type]);
            rb_iv_set(self, "@tx_handle", Qnil);            
        }
        break;
        
    case LORA_MAC_RX:
    
        if(rx_handle != Qnil){
    
            rb_funcall(rx_handle, rb_intern("call"), 2, UINT2NUM(arg->rx.port), rb_str_new((const char *)arg->rx.data, arg->rx.len));
        }
        break;
    }   
}

static VALUE bw_to_number(enum lora_signal_bandwidth bw)
{
    return UINT2NUM(bw);
}

static VALUE sf_to_number(enum lora_spreading_factor sf)
{
    return UINT2NUM(sf);
}


static VALUE cr_to_number(enum lora_coding_rate cr)
{
    return UINT2NUM(cr);
}

static enum lora_signal_bandwidth number_to_bw(VALUE bw)
{
    enum lora_signal_bandwidth retval;
    size_t i;
    
    static const enum lora_signal_bandwidth map[] = {
        BW_125,
        BW_250,
        BW_500
    };
    
    for(i=0U; i < sizeof(map)/sizeof(*map); i++){
        
        if(map[i] == NUM2UINT(bw)){
           
            retval = map[i];            
            break;
        }        
    }
    
    if(i == sizeof(map)/sizeof(*map)){
        
        rb_raise(rb_eRangeError, "not a valid bandwidth");
    }
    
    return retval;
}

static enum lora_spreading_factor number_to_sf(VALUE sf)
{
    enum lora_spreading_factor retval;
    size_t i;
    
    static const enum lora_spreading_factor map[] = {
        SF_7,
        SF_8,
        SF_9,
        SF_10,
        SF_11,
        SF_12
    };
    
    for(i=0U; i < sizeof(map)/sizeof(*map); i++){
        
        if(map[i] == NUM2UINT(sf)){
           
            retval = map[i];            
            break;
        }        
    }
    
    if(i == sizeof(map)/sizeof(*map)){
        
        rb_raise(rb_eRangeError, "not a valid spreading factor");
    }
    
    return retval;
}

static VALUE io_event(VALUE self, VALUE event, VALUE time)
{
    size_t i;
    struct lora_mac *this;    
    Data_Get_Struct(self, struct lora_mac, this);
    
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

static VALUE ticksUntilNextEvent(VALUE self)
{
    struct lora_mac *this;    
    uint64_t next;
    Data_Get_Struct(self, struct lora_mac, this);
    
    next = MAC_ticksUntilNextEvent(this);
    
    return (next == UINT64_MAX) ? Qnil : ULL2NUM(next);
}

static VALUE transmitTimeUp(VALUE self, VALUE bandwidth, VALUE spreading_factor, VALUE size)
{
    return UINT2NUM(MAC_transmitTimeUp(number_to_bw(bandwidth), number_to_sf(spreading_factor), (uint8_t)NUM2UINT(size)));
}

static VALUE transmitTimeDown(VALUE self, VALUE bandwidth, VALUE spreading_factor, VALUE size)
{
    return UINT2NUM(MAC_transmitTimeDown(number_to_bw(bandwidth), number_to_sf(spreading_factor), (uint8_t)NUM2UINT(size)));
}

static VALUE ticksUntilNextChannel(VALUE self)
{
    struct lora_mac *this;    
    uint64_t next;
    Data_Get_Struct(self, struct lora_mac, this);
    
    next = MAC_ticksUntilNextChannel(this);
    
    return (next == UINT64_MAX) ? Qnil : ULL2NUM(next);
}
