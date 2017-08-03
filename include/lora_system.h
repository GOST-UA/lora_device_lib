#ifndef LORA_SYSTEM_H
#define LORA_SYSTEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

uint64_t System_getTime(void);

void System_usleep(uint32_t interval);

#ifdef __cplusplus
}
#endif

#endif
