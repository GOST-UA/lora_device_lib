#ifndef LORA_SYSTEM_H
#define LORA_SYSTEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

uint64_t System_getTime(void);

void System_usleep(uint32_t interval);

void System_atomic_setPtr(void **receiver, void *value);

void System_rand(uint8_t *data, size_t len);

void System_getAppEUI(void *owner, uint8_t *eui);
void System_getDevEUI(void *owner, uint8_t *eui);
void System_getAppKey(void *owner, uint8_t *key);

#ifdef __cplusplus
}
#endif

#endif
