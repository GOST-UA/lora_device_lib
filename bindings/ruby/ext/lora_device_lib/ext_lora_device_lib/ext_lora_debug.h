#ifndef EXT_LORA_DEBUG_H
#define EXT_LORA_DEBUG_H

#include <stdio.h>
#include <assert.h>
#include <ruby.h>

/* LORA_ERROR will log as 'info' */
#define LORA_ERROR(...) \
    do{\
        VALUE args[] = {\
            rb_funcall(rb_cFile, rb_intern("basename"), 1, rb_str_new_cstr(__FILE__)),\
            UINT2NUM(__LINE__),\
            rb_str_new_cstr(__FUNCTION__),\
            rb_sprintf(__VA_ARGS__)\
        };\
        VALUE msg = rb_str_format(sizeof(args)/sizeof(*args), args, rb_str_new_cstr("%s: %u: %s(): %s"));\
        rb_funcall(self, rb_intern("log_error"), 1, msg);\
    }while(0);

/* LORA_ASSERT will raise a LoraDeviceLib::LoraAssert */
#define LORA_ASSERT(X) \
    if(!(X)){\
        VALUE args[] = {\
            rb_funcall(rb_cFile, rb_intern("basename"), 1, rb_str_new_cstr(__FILE__)),\
            UINT2NUM(__LINE__),\
            rb_str_new_cstr(__FUNCTION__),\
            rb_str_new_cstr(#X)\
        };\
        VALUE msg = rb_str_format(sizeof(args)/sizeof(*args), args, rb_str_new_cstr("%s: %u: %s(): assertion failed: %s"));\
        VALUE ex = rb_funcall(rb_const_get(rb_const_get(rb_cObject, rb_intern("LoraDeviceLib")), rb_intern("LoraError")), rb_intern("new"), 1, msg);\
        rb_funcall(rb_mKernel, rb_intern("raise"), 1, ex);\
    }

/* LORA_MESSAGE will log as 'info' */
#define LORA_MESSAGE(...) \
    do{\
        VALUE args[] = {\
            rb_funcall(rb_cFile, rb_intern("basename"), 1, rb_str_new_cstr(__FILE__)),\
            rb_str_new_cstr(__FUNCTION__),\
            rb_sprintf(__VA_ARGS__)\
        };\
        VALUE msg = rb_str_format(sizeof(args)/sizeof(*args), args, rb_str_new_cstr("%s: %s(): %s"));\
        rb_funcall(self, rb_intern("log_info"), 1, msg);\
    }while(0);


#endif
