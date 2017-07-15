/* Copyright (c) 2017 Cameron Harper
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * */

#include "lora_region.h"
#include "lora_debug.h"

struct lora_region_channel {

    uint32_t freq;          /**< carrier frequency */
    const uint8_t *rates;   /**< permitted data rates */
    size_t num_rates;
    uint8_t chIndex;        
};

/** a range of frequencies ( begin .. end ) */
struct lora_region_band {    
    uint32_t begin; /**< Hz */
    uint32_t end;   /**< Hz */
    uint8_t id;     /**< several bands may be combined into one by id */
};

struct lora_region {

    enum lora_region_id id;

    const struct lora_data_rate **rates;    /**< data rate definitions (upstream and downstream) */     
    size_t num_rates;                       /**< number of entries in `rates` */
    
    const uint8_t *rx1_rate;            /**< pointer to flat array of permitted RX1 data rates */
    uint8_t rx1_row_size;               /**< `rx1_rate` row size */
    uint8_t rx1_col_size;               /**< 'rx1_rate' column size */
    
    const struct lora_region_band *bands;   /**< collection of frequency band definitions which map to "duty cycle bands" */
    size_t num_bands;                       /**< number of entries in `bands` */

    const uint16_t *off_time_factors;   /**< collection of off-time factors indexed per "duty cycle band" */
    size_t num_off_time_factors;
    
    const struct lora_region_channel *default_channels;    /**< some regions have default channels */
    size_t num_default_channels;                    /**< number of entries in `default_channels` */

    const struct lora_region_default *defaults;     /**< other default settings for this region */
};

/**********************************************************************/
/* EU_863_870: */

static const struct lora_data_rate dr0_EU_863_870 = {
    .sf = SF_12,
    .bw = BW_125,
    .payload = 59U,
};

static const struct lora_data_rate dr1_EU_863_870 = {
    .sf = SF_11,
    .bw = BW_125,
    .payload = 59U,
};
    
static const struct lora_data_rate dr2_EU_863_870 = {
    .sf = SF_10,
    .bw = BW_125,
    .payload = 59U,        
};
    
static const struct lora_data_rate dr3_EU_863_870 = {
    .sf = SF_9,
    .bw = BW_125,
    .payload = 123U,        
};
    
static const struct lora_data_rate dr4_EU_863_870 = {
    .sf = SF_8,
    .bw = BW_125,
    .payload = 230U,        
};

static const struct lora_data_rate dr5_EU_863_870 = {
    .sf = SF_7,
    .bw = BW_125,
    .payload = 230U,        
};
    
static const struct lora_data_rate dr6_EU_863_870 = {
    .sf = SF_7,
    .bw = BW_250,
    .payload = 230U,        
};

static const struct lora_data_rate *rates_EU_863_870[] = {
    &dr0_EU_863_870,
    &dr1_EU_863_870,
    &dr2_EU_863_870,
    &dr3_EU_863_870,
    &dr4_EU_863_870,
    &dr5_EU_863_870,
    &dr6_EU_863_870,
};

static const uint8_t erp_EU_863_870[] = {
    20U,
    14U,
    11U,
    8U,
    5U,
    2U
};

static const uint16_t off_time_factors_EU_863_870[] = {
    100U,    // 1.0% (band 0)
    100U,    // 1.0% (band 1)    
    1000U,   // 0.1% (band 2)
    10U,     // 10.0% (band 3)
    100U,    // 1.0% (band 4)
};

static const struct lora_region_band bands_EU_863_870[] = {
    {
        .begin = 863000000U,
        .end = 865000000U,
        .id = 2
    },
    {
        .begin = 865000000U,
        .end = 868000000U,
        .id = 0
    },
    {
        .begin = 868000000U,
        .end = 868600000U,
        .id = 1
    },
    {
        .begin = 868700000U,
        .end = 869200000U,
        .id = 2
    },
    {
        .begin = 868700000U,
        .end = 869200000U,
        .id = 3
    },
    {
        .begin = 868700000U,
        .end = 869200000U,
        .id = 4
    }
    };

static const uint8_t default_channel_rates_EU_863_870[] = {
    0U,  // DR0
    1U,  // DR1
    2U,  // DR2
    3U,  // DR3
    4U,  // DR4
    5U,  // DR5
    6U,  // DR6
    7U   // DR7
};

static const uint8_t rx1_rate_EU_863_870[] = {
    0U, 0U, 0U, 0U, 0U, 0U,   // DR0 upstream
    1U, 0U, 0U, 0U, 0U, 0U,   // DR1 upstream
    2U, 1U, 0U, 0U, 0U, 0U,   // DR2 upstream
    3U, 2U, 1U, 0U, 0U, 0U,   // DR3 upstream
    4U, 3U, 2U, 1U, 0U, 0U,   // DR4 upstream
    5U, 4U, 3U, 2U, 1U, 0U,   // DR5 upstream
    6U, 5U, 4U, 3U, 2U, 1U,   // DR6 upstream
    7U, 6U, 5U, 4U, 3U, 2U,   // DR7 upstream 
};

static const struct lora_region_channel default_channels_EU_863_870[] = {
    {
        .freq = 868100000U,
        .rates = default_channel_rates_EU_863_870,
        .num_rates = sizeof(default_channel_rates_EU_863_870)/sizeof(*default_channel_rates_EU_863_870),
        .chIndex = 0U
    },
    {
        .freq = 868300000U,
        .rates = default_channel_rates_EU_863_870,
        .num_rates = sizeof(default_channel_rates_EU_863_870)/sizeof(*default_channel_rates_EU_863_870),
        .chIndex = 1U
    },
    {
        .freq = 868500000U,
        .rates = default_channel_rates_EU_863_870,
        .num_rates = sizeof(default_channel_rates_EU_863_870)/sizeof(*default_channel_rates_EU_863_870),
        .chIndex = 2U
    }
};

static const struct lora_region_default defaults_EU_863_870 = {
    
    .max_fcnt_gap = 16384U,
    
    .rx1_delay = 1U,
    .rx2_delay = 2U,
    
    .ja1_delay = 5U,
    .ja2_delay = 6U,
    
    .adr_ack_delay = 32U,
    .adr_ack_limit = 64U,
    .adr_ack_timeout = 2U,
    .adr_ack_dither = 1U,
    
    .rx2_freq = 869525000U,
    .rx2_rate = 0U,
    
    .init_tx_rate = 6U,
    .init_tx_power = 0U,
};

static const struct lora_region info_EU_863_870 = {

    .id = EU_863_870,

    .rates = rates_EU_863_870,
    .num_rates = sizeof(rates_EU_863_870)/sizeof(*rates_EU_863_870),
    
    .rx1_rate = rx1_rate_EU_863_870,
    .rx1_row_size = 6U,
    .rx1_col_size = (sizeof(rx1_rate_EU_863_870)/6U),
    
    .default_channels = default_channels_EU_863_870,
    .num_default_channels = sizeof(default_channels_EU_863_870)/sizeof(*default_channels_EU_863_870),

    .defaults = &defaults_EU_863_870,

    .bands = bands_EU_863_870,
    .num_bands = sizeof(bands_EU_863_870)/sizeof(*bands_EU_863_870),

    .off_time_factors = off_time_factors_EU_863_870,
    .num_off_time_factors = sizeof(off_time_factors_EU_863_870) / sizeof(*off_time_factors_EU_863_870),    
};

/**********************************************************************/

static const struct lora_region *regions[] = {
    &info_EU_863_870,
    NULL, // US_902_928,
    NULL, // CN_779_787,
    NULL, // EU_433,
    NULL, // AUSTRALIA_915_928,
    NULL, // CN_470_510,
    NULL, // AS_923,
    NULL, // KR_920_923,
};

/**********************************************************************/

const struct lora_region *Region_getRegion(enum lora_region_id region)
{
    return regions[region];    
}

const struct lora_data_rate *Region_getDataRateParameters(const struct lora_region *self, uint8_t rate)
{
    LORA_ASSERT(self != NULL)
    
    const struct lora_data_rate *retval = NULL;

    if(rate < self->num_rates){

        retval = self->rates[rate];
    }
    
    return retval;
}

bool Region_validateFrequency(const struct lora_region *self, uint32_t frequency, uint8_t *band)
{
    LORA_ASSERT(self != NULL)
    LORA_ASSERT(band != NULL)
    
    bool retval = false;
    size_t i;

    for(i=0U; i < self->num_bands; i++){

        if((frequency >= self->bands[i].begin) && (frequency <= self->bands[i].end)){

            retval = true;
            *band = self->bands[i].id;
            break;
        }
    }
    

    return retval;
}

bool Region_validateRate(const struct lora_region *self, uint32_t frequency, uint8_t rate)
{
    LORA_ASSERT(self != NULL)
    
    bool retval = false;
    uint8_t band;

    if(Region_validateFrequency(self, frequency, &band)){

        retval = true;
    }

    return retval;
}

bool Region_getOffTimeFactor(const struct lora_region *self, uint8_t band, uint16_t *offtime_factor)
{
    LORA_ASSERT(self != NULL)
    LORA_ASSERT(offtime_factor != NULL)
    
    bool retval = false;

    if(band < self->num_off_time_factors){

        *offtime_factor = self->off_time_factors[band];
        retval = true;
    }

    return retval;
}

const struct lora_region_default *Region_getDefaultSettings(const struct lora_region *self)
{
    LORA_ASSERT(self != NULL)
    
    return self->defaults;
}

void Region_getDefaultChannels(const struct lora_region *self, void *receiver, void (*handler)(void *reciever, uint8_t chIndex, uint32_t freq))
{
    LORA_ASSERT(self != NULL)
    LORA_ASSERT(handler != NULL)
    
    size_t i;
    
    for(i=0U; i < self->num_default_channels; i++){
        
        handler(receiver, self->default_channels[i].chIndex, self->default_channels[i].freq);
    }    
}

bool Region_getRX1DataRate(const struct lora_region *self, uint8_t tx_rate, uint8_t rx1_offset, uint8_t *rx1_rate)
{
    LORA_ASSERT(self != NULL)
    LORA_ASSERT(rx1_rate != NULL)

    bool retval = false;
    uint16_t index;
    
    if(rx1_offset < self->rx1_row_size){
        
        if(tx_rate < self->rx1_col_size){
        
            index = tx_rate * self->rx1_row_size;
            index += rx1_offset;
            
            *rx1_rate = self->rx1_rate[index];
            
            retval = true;
        }    
    }
    
    return retval;
}
