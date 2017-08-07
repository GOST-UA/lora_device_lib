#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"

#include "lora_system.h"

uint64_t System_getTime(void)
{
    return mock_type(uint64_t);
}

void System_usleep(uint32_t interval)
{
}
