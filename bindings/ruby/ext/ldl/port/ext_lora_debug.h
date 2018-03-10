/* Copyright (c) 2017-2018 Cameron Harper
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

#ifndef EXT_LORA_DEBUG_H
#define EXT_LORA_DEBUG_H

#include <ruby.h>

/* LORA_ERROR will log as 'error' */
#define LORA_ERROR(...) \
    do{\
        VALUE args[] = {\
            rb_funcall(rb_cFile, rb_intern("basename"), 1, rb_str_new_cstr(__FILE__)),\
            UINT2NUM(__LINE__),\
            rb_str_new_cstr(__FUNCTION__),\
            rb_sprintf(__VA_ARGS__)\
        };\
        VALUE msg = rb_str_format(sizeof(args)/sizeof(*args), args, rb_str_new_cstr("%s: %u: %s(): %s"));\
        VALUE ex = rb_funcall(rb_const_get(rb_const_get(rb_cObject, rb_intern("LDL")), rb_intern("Error")), rb_intern("new"), 1, msg);\
        rb_funcall(rb_mKernel, rb_intern("raise"), 1, ex);\
    }while(0);

/* LORA_INFO will log as 'info' */
#define LORA_INFO(...) \
    do{\
        VALUE args[] = {\
            ULL2NUM(System_time()),\
            rb_funcall(rb_cFile, rb_intern("basename"), 1, rb_str_new_cstr(__FILE__)),\
            rb_str_new_cstr(__FUNCTION__),\
            rb_sprintf(__VA_ARGS__)\
        };\
        VALUE msg = rb_str_format(sizeof(args)/sizeof(*args), args, rb_str_new_cstr("(%u) %s: %s(): %s"));\
        rb_funcall(rb_const_get(rb_const_get(rb_cObject, rb_intern("LDL")), rb_intern("Logger")), rb_intern("info"), 1, msg);\
    }while(0);

/* LORA_DEBUG will log as 'debug' */
#define LORA_DEBUG(...) \
    do{\
        VALUE args[] = {\
            ULL2NUM(System_time()),\
            rb_funcall(rb_cFile, rb_intern("basename"), 1, rb_str_new_cstr(__FILE__)),\
            rb_str_new_cstr(__FUNCTION__),\
            rb_sprintf(__VA_ARGS__)\
        };\
        VALUE msg = rb_str_format(sizeof(args)/sizeof(*args), args, rb_str_new_cstr("(%u) %s: %s(): %s"));\
        rb_funcall(rb_const_get(rb_const_get(rb_cObject, rb_intern("LDL")), rb_intern("Logger")), rb_intern("debug"), 1, msg);\
    }while(0);

/* LORA_ASSERT will raise a LDL::LoraAssert */
#define LORA_ASSERT(X) \
    if(!(X)){\
        VALUE args[] = {\
            rb_funcall(rb_cFile, rb_intern("basename"), 1, rb_str_new_cstr(__FILE__)),\
            UINT2NUM(__LINE__),\
            rb_str_new_cstr(__FUNCTION__),\
            rb_str_new_cstr(#X)\
        };\
        VALUE msg = rb_str_format(sizeof(args)/sizeof(*args), args, rb_str_new_cstr("%s: %u: %s(): assertion failed: %s"));\
        VALUE ex = rb_funcall(rb_const_get(rb_const_get(rb_cObject, rb_intern("LDL")), rb_intern("LoraAssert")), rb_intern("new"), 1, msg);\
        rb_funcall(rb_mKernel, rb_intern("raise"), 1, ex);\
    }

/* LORA_PEDANTIC will expand to LORA_ASSERT */
#define LORA_PEDANTIC(X) LORA_ASSERT(X)


#endif
