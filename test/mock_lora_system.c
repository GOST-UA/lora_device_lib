#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"

#include "lora_system.h"

#include <string.h>

uint64_t System_getTime(void)
{
    return mock_type(uint64_t);
}

void System_usleep(uint32_t interval)
{
}

void System_atomic_setPtr(void **receiver, void *value)
{
    *receiver = value;
}

void System_getAppEUI(void *owner, void *eui)
{
    (void)memcpy(eui, mock_ptr_type(uint8_t *), 8U);
}

void System_getDevEUI(void *owner, void *eui)
{
    (void)memcpy(eui, mock_ptr_type(uint8_t *), 8U);
}

void System_getAppKey(void *owner, void *key)
{
    (void)memcpy(key, mock_ptr_type(uint8_t *), 16U);
}
