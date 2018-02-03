#include <stdint.h>

uint64_t system_time = 0U;

uint64_t System_getTime(void)
{
    return system_time;
}
