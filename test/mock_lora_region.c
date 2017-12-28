#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "cmocka.h"
#include "lora_region.h"

bool Region_getRate(const struct lora_region *self, uint8_t rate, struct lora_data_rate *setting)
{
    (void)memset(setting, 0, sizeof(*setting));
    
    return true;
}

void Region_getDefaultChannels(const struct lora_region *self, void *receiver, void (*handler)(void *reciever, uint8_t chIndex, uint32_t freq, uint8_t minRate, uint8_t maxRate))
{
}

bool Region_validateFrequency(const struct lora_region *self, uint32_t frequency, uint8_t *band)
{
    bool retval = false;
    
    if(mock_type(bool)){

        *band = mock();
        retval = true;
    }

    return retval;
}

uint16_t Region_getOffTimeFactor(const struct lora_region *self, uint8_t band)
{
    return mock();
}

bool Region_validateRate(const struct lora_region *self, uint8_t chIndex, uint8_t minRate, uint8_t maxRate)
{
    return mock();
}

void Region_getDefaultSettings(const struct lora_region *self, struct lora_region_default *defaults)
{
    memset(defaults, 0, sizeof(*defaults));
}

bool Region_getRX1DataRate(const struct lora_region *self, uint8_t tx_rate, uint8_t rx1_offset, uint8_t *rx1_rate)
{
    *rx1_rate = mock();
    
    return mock();
}
