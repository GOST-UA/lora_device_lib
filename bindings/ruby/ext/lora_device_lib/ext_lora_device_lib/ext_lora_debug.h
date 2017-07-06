#ifndef EXT_LORA_DEBUG_H
#define EXT_LORA_DEBUG_H

#include <stdio.h>
#include <assert.h>
#include <ruby.h>

#define LORA_ERROR(...) do{fprintf(stderr, "%s: %u: ", __FUNCTION__, __LINE__);fprintf(stderr, __VA_ARGS__);fprintf(stderr, "\n");}while(0);
#define LORA_MESSAGE(...) do{fprintf(stderr, "%s: %u: ", __FUNCTION__, __LINE__);fprintf(stderr, __VA_ARGS__);fprintf(stderr, "\n");}while(0);
#define LORA_ASSERT(X) assert((X));

#endif
