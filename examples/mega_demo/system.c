#include "lora_system.h"

#include <util/atomic.h>

#include <stdlib.h>

uint64_t System_getTime(void)
{
    return 0U;
}

void System_usleep(uint32_t interval)
{
}

void System_atomic_setPtr(void **receiver, void *value)
{
    ATOMIC_BLOCK(ATOMIC_FORCEON)
    {
        *receiver = value;
    }    
}

uint8_t System_rand(void)
{
    return (uint8_t)rand();
}

uint8_t System_getBatteryLevel(void *owner)
{
    return 255U;    // not available
}
