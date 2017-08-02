#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdbool.h>

#include "cmocka.h"
#include "lora_region.h"

const struct lora_data_rate *Region_getDataRateParameters(const struct lora_region *self, uint8_t rate)
{
    return mock_ptr_type(struct lora_data_rate *);
}

void Region_getDefaultChannels(const struct lora_region *self, void *receiver, void (*cb)(void *reciever, uint8_t chIndex, uint32_t freq))
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

bool Region_getOffTimeFactor(const struct lora_region *self, uint8_t band, uint16_t *offtime_factor)
{
    *offtime_factor = mock();
    
    return mock();
}

bool Region_validateRate(const struct lora_region *self, uint8_t chIndex, uint8_t rate)
{
    return mock();
}

uint8_t Region_getTXRates(const struct lora_region *self, uint8_t chIndex, const uint8_t **rates)
{
    *rates = mock_ptr_type(const uint8_t *);    
    return mock();
}

const struct lora_region_default *Region_getDefaultSettings(const struct lora_region *self)
{
    return mock_ptr_type(const struct lora_region_default *);
}
