#ifndef DEBUG_INCLUDE_H
#define DEBUG_INCLUDE_H

#include <stdio.h>
#include <assert.h>

#define LORA_ERROR(...) do{fprintf(stderr, "%s: %u: ", __FUNCTION__, __LINE__);fprintf(stderr, __VA_ARGS__);fprintf(stderr, "\n");}while(0);

#define LORA_ASSERT(X) assert((X));


#endif
