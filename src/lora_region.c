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

/* definitions ********************************************************/

#include "lora_region.h"
#include "lora_debug.h"

#if defined(LORA_AVR)

    #include <avr/pgmspace.h>
    
#else

    #include <string.h>
    
    #define PROGMEM
    #define memcpy_P memcpy
    
#endif

struct lora_region_channel {

    uint32_t freq : 24U; /**< x 10^2 Hz */           
    uint8_t chIndex;
};

/** a range of frequencies ( begin .. end ) */
struct lora_region_band {    
    uint32_t begin : 24U;   /**< x 10^2 Hz */
    uint32_t end : 24U;     /**< x 10^2 Hz */
    uint8_t id;             /**< several bands may be combined into one by id */
};

struct lora_region_rate_allocation {
    
    uint8_t minChIndex;
    uint8_t maxChIndex;        
    uint8_t minRate;
    uint8_t maxRate;
};

struct lora_region {

    enum lora_region_id id;

    const struct lora_data_rate *rates;    /**< data rate definitions (upstream and downstream) */     
    uint8_t num_rates;                       /**< number of entries in `rates` */
    
    const uint8_t *rx1_rate;            /**< pointer to flat array of permitted RX1 data rates */
    uint8_t rx1_row_size;               /**< `rx1_rate` row size */
    uint8_t rx1_col_size;               /**< 'rx1_rate' column size */
    
    const struct lora_region_band *bands;   /**< collection of frequency band definitions which map to "duty cycle bands" */
    uint8_t num_bands;                       /**< number of entries in `bands` */

    const uint16_t *off_time_factors;   /**< collection of off-time factors indexed per "duty cycle band" */
    uint8_t num_off_time_factors;
    
    const struct lora_region_rate_allocation *rate_allocation;
    size_t num_rate_allocation;
    
    const struct lora_region_channel *default_channels;    /**< some regions have default channels */
    uint8_t num_default_channels;                    /**< number of entries in `default_channels` */

    const struct lora_region_default *defaults;     /**< other default settings for this region */
    
    uint8_t numChannels;                            /**< how many channels in this region? */
};

/* static function prototypes *****************************************/

static bool getRateRange(const struct lora_region *self, uint8_t chIndex, uint8_t *minRate, uint8_t *maxRate);
static bool freqToChannel(const struct lora_region *self, uint32_t freq, uint8_t *chIndex);

/* data ***************************************************************/

static const struct lora_data_rate rates_EU_863_870[] PROGMEM = {
    {
        .sf = SF_12,
        .bw = BW_125,
        .payload = 59U,
        .rate = 0U
    },
    {
        .sf = SF_11,
        .bw = BW_125,
        .payload = 59U,
        .rate = 1U
    },
    {
        .sf = SF_10,
        .bw = BW_125,
        .payload = 59U,        
        .rate = 2U
    },
    {
        .sf = SF_9,
        .bw = BW_125,
        .payload = 123U,        
        .rate = 3U
    },
    {
        .sf = SF_8,
        .bw = BW_125,
        .payload = 230U,        
        .rate = 4U
    },
    {
        .sf = SF_7,
        .bw = BW_125,
        .payload = 230U,       
        .rate = 5U 
    },
    {
        .sf = SF_7,
        .bw = BW_250,
        .payload = 230U,        
        .rate = 6U
    }
};

static const struct lora_data_rate rates_US_902_928[] PROGMEM = {
    {
        .sf = SF_10,
        .bw = BW_125,
        .payload = 19U,
        .rate = 0U
    },
    {
        .sf = SF_9,
        .bw = BW_125,
        .payload = 61U,
        .rate = 1U
    },
    {
        .sf = SF_8,
        .bw = BW_125,
        .payload = 133U,        
        .rate = 2U
    },
    {
        .sf = SF_7,
        .bw = BW_125,
        .payload = 250U,        
        .rate = 3U
    },
    {
        .sf = SF_8,
        .bw = BW_500,
        .payload = 41U,        
        .rate = 4U
    },    
    {
        .sf = SF_12,
        .bw = BW_500,
        .payload = 117U,       
        .rate = 8U 
    },
    {
        .sf = SF_11,
        .bw = BW_500,
        .payload = 230U,       
        .rate = 9U 
    },
    {
        .sf = SF_10,
        .bw = BW_500,
        .payload = 230U,       
        .rate = 10U 
    },
    {
        .sf = SF_9,
        .bw = BW_500,
        .payload = 230U,       
        .rate = 11U 
    },
    {
        .sf = SF_8,
        .bw = BW_500,
        .payload = 230U,       
        .rate = 12U 
    },
    {
        .sf = SF_7,
        .bw = BW_500,
        .payload = 230U,       
        .rate = 13U 
    }
};

static const uint8_t erp_EU_863_870[] PROGMEM = {
    20U,
    14U,
    11U,
    8U,
    5U,
    2U
};

static const uint8_t erp_US_902_928[] PROGMEM = {
    30U,
    28U,
    26U,
    0U,
    0U,
    0U,
    0U,
    0U,
    0U,
    0U,
    10U    
};

static const uint16_t off_time_factors_EU_863_870[] PROGMEM = {
    100U,    // 1.0% (band 0)
    100U,    // 1.0% (band 1)    
    1000U,   // 0.1% (band 2)
    10U,     // 10.0% (band 3)
    100U,    // 1.0% (band 4)
};

static const uint16_t off_time_factors_US_902_928[] PROGMEM = {
    1U
};

static const struct lora_region_band bands_EU_863_870[] PROGMEM = {
    {
        .begin = 8630000U,
        .end = 8650000U,
        .id = 2,
    },
    {
        .begin = 8650000U,
        .end = 8680000U,
        .id = 0,
    },
    {
        .begin = 8680000U,
        .end = 8686000U,
        .id = 1,
    },
    {
        .begin = 8687000U,
        .end = 8692000U,
        .id = 2,
    },
    {
        .begin = 8687000U,
        .end = 8692000U,
        .id = 3,
    },
    {
        .begin = 8687000U,
        .end = 8692000U,
        .id = 4,
    }
};

/* I can't find any infomration about this */
static const struct lora_region_band bands_US_902_928[] PROGMEM = {    
    {
        .begin = 9020000U,
        .end = 9280000U,
        .id = 0,
    }
};

static const uint8_t rx1_rate_EU_863_870[] PROGMEM = {
    0U, 0U, 0U, 0U, 0U, 0U,   // DR0 upstream
    1U, 0U, 0U, 0U, 0U, 0U,   // DR1 upstream
    2U, 1U, 0U, 0U, 0U, 0U,   // DR2 upstream
    3U, 2U, 1U, 0U, 0U, 0U,   // DR3 upstream
    4U, 3U, 2U, 1U, 0U, 0U,   // DR4 upstream
    5U, 4U, 3U, 2U, 1U, 0U,   // DR5 upstream
    6U, 5U, 4U, 3U, 2U, 1U,   // DR6 upstream
    7U, 6U, 5U, 4U, 3U, 2U,   // DR7 upstream 
};

static const uint8_t rx1_rate_US_902_928[] PROGMEM = {
    10U, 9U,  8U,  8U,      // DR0 upstream
    11U, 10U, 9U,  8U,      // DR1 upstream
    12U, 11U, 10U, 9U,      // DR2 upstream
    13U, 12U, 11U, 10U,     // DR3 upstream
    13U, 13U, 12U, 11U,     // DR4 upstream
};

static const struct lora_region_rate_allocation rate_allocation_EU_863_870[] PROGMEM = {
    {
        .minChIndex = 0U,
        .maxChIndex = 15U,        
        .minRate = 0U,
        .maxRate = 7U
    }    
};

static const struct lora_region_rate_allocation rate_allocation_US_902_928[] PROGMEM = {
    {
        .minChIndex = 0U,
        .maxChIndex = 63U,        
        .minRate = 0U,
        .maxRate = 3U
    },
    {
        .minChIndex = 64U,
        .maxChIndex = 71U,        
        .minRate = 4U,
        .maxRate = 4U
    }
};

static const struct lora_region_channel default_channels_EU_863_870[] PROGMEM = {
    {
        .freq = 8681000U,
        .chIndex = 0U
    },
    {
        .freq = 8683000U,
        .chIndex = 1U
    },
    {
        .freq = 8685000U,
        .chIndex = 2U
    }
};

static const struct lora_region_default defaults_EU_863_870 PROGMEM = {
    
    .max_fcnt_gap = 16384U,
    
    .rx1_delay = 1U,    
    .ja1_delay = 5U,
    
    .rx1_offset = 0U,
    
    .rx2_freq = 869525000U,
    .rx2_rate = 0U,
    
    .adr_ack_delay = 32U,
    .adr_ack_limit = 64U,
    .adr_ack_timeout = 2U,
    .adr_ack_dither = 1U,
    
    .tx_rate = 6U,
    .tx_power = 0U,
};

static const struct lora_region_default defaults_US_902_928 PROGMEM = {
    
    .max_fcnt_gap = 16384U,
    
    .rx1_delay = 1U,
    
    .ja1_delay = 5U,
    
    .rx1_offset = 0U,
    
    .rx2_freq = 923300000U,
    .rx2_rate = 8U,
    
    .adr_ack_delay = 32U,
    .adr_ack_limit = 64U,
    .adr_ack_timeout = 2U,
    .adr_ack_dither = 1U,
    
    .tx_rate = 6U,
    .tx_power = 0U,
};


static const struct lora_region regions[] = {
    
    /* EU_863_870 */
    {
        .id = EU_863_870,

        .rates = rates_EU_863_870,
        .num_rates = (uint8_t)sizeof(rates_EU_863_870)/sizeof(*rates_EU_863_870),
        
        .rx1_rate = rx1_rate_EU_863_870,
        .rx1_row_size = 6U,
        .rx1_col_size = (uint8_t)(sizeof(rx1_rate_EU_863_870)/6U),
        
        .default_channels = default_channels_EU_863_870,
        .num_default_channels = (uint8_t)sizeof(default_channels_EU_863_870)/sizeof(*default_channels_EU_863_870),

        .defaults = &defaults_EU_863_870,

        .bands = bands_EU_863_870,
        .num_bands = (uint8_t)sizeof(bands_EU_863_870)/sizeof(*bands_EU_863_870),

        .off_time_factors = off_time_factors_EU_863_870,
        .num_off_time_factors = (uint8_t)sizeof(off_time_factors_EU_863_870) / sizeof(*off_time_factors_EU_863_870),    
        
        .rate_allocation = rate_allocation_EU_863_870,
        .num_rate_allocation = sizeof(rate_allocation_EU_863_870)/sizeof(*rate_allocation_EU_863_870),
        
        .numChannels = 16U
    },
    /* US_902_928 */
    {
        .id = US_902_928,

        .rates = rates_US_902_928,
        .num_rates = (uint8_t)sizeof(rates_US_902_928)/sizeof(*rates_US_902_928),
        
        .rx1_rate = rx1_rate_US_902_928,
        .rx1_row_size = 4U,
        .rx1_col_size = (uint8_t)(sizeof(rx1_rate_US_902_928)/4U),
        
        .default_channels = NULL,
        .num_default_channels = 0U,

        .rate_allocation = rate_allocation_US_902_928,
        .num_rate_allocation = sizeof(rate_allocation_US_902_928)/sizeof(*rate_allocation_US_902_928),

        .defaults = &defaults_US_902_928,

        .bands = bands_US_902_928,
        .num_bands = (uint8_t)sizeof(bands_US_902_928)/sizeof(*bands_US_902_928),

        .off_time_factors = off_time_factors_US_902_928,
        .num_off_time_factors = (uint8_t)sizeof(off_time_factors_US_902_928) / sizeof(*off_time_factors_US_902_928),    
        
        .rate_allocation = rate_allocation_US_902_928,
        .num_rate_allocation = (uint8_t)sizeof(rate_allocation_US_902_928)/sizeof(*rate_allocation_US_902_928),
        
        .numChannels = 72U
    }         
};




/* functions **********************************************************/

const struct lora_region *Region_getRegion(enum lora_region_id region)
{
    size_t i;
    const struct lora_region *retval = NULL;
    
    for(i=0U; i < sizeof(regions)/sizeof(*regions); i++){
    
        if(regions[i].id == region){
    
            retval = &regions[i];
            break;
        }
    }
    
    return retval;    
}

void Region_getDefaultSettings(const struct lora_region *self, struct lora_region_default *defaults)
{
    LORA_PEDANTIC(self != NULL)    
    (void)memcpy_P(defaults, self->defaults, sizeof(*defaults));
}

bool Region_getRate(const struct lora_region *self, uint8_t rate, struct lora_data_rate *setting)
{
    LORA_PEDANTIC(self != NULL)
    LORA_PEDANTIC(setting != NULL)
    
    size_t i;
    uint8_t r;
    bool retval = false;
    
    for(i=0U; i < self->num_rates; i++){
        
        (void)memcpy_P(&r, &self->rates[i].rate, sizeof(r));
        
        if(r == rate){
        
            (void)memcpy_P(setting, &self->rates[i], sizeof(*setting));
            retval = true;
        }        
    }    
   
    return retval;
}

bool Region_getBand(const struct lora_region *self, uint32_t freq, uint8_t *band)
{
    LORA_PEDANTIC(self != NULL)
    LORA_PEDANTIC(band != NULL)
    
    bool retval = false;
    uint8_t i;

    for(i=0U; i < self->num_bands; i++){

        struct lora_region_band setting;
        
        (void)memcpy_P(&setting, &self->bands[i], sizeof(band));

        if((freq >= (setting.begin * 100U)) && (freq <= (setting.end * 100U))){

            retval = true;
            *band = setting.id;
            break;
        }
    }

    return retval;
}

bool Region_isDynamic(const struct lora_region *self)
{
    bool retval;
    
    switch(self->id){
    case EU_863_870:
    case EU_433:
    case AS_923:
    case KR_920_923:
    case CN_779_787:                        
        retval = true;
        break;    
    default:                    
    case US_902_928:                    
    case AU_915_928:
    case CN_470_510:
        retval = false;                    
        break;
    }
    
    return retval;
}

bool Region_getChannel(const struct lora_region *self, uint8_t chIndex, uint32_t *freq, uint8_t *minRate, uint8_t *maxRate)
{
    bool retval = false;
    
    switch(self->id){
    default:
    case EU_863_870:
    case EU_433:
    case AS_923:
    case KR_920_923:
    case CN_779_787:  
        break;                      
    case US_902_928:            
    case AU_915_928:
    
        retval = true;
        
        if(chIndex < 64U){
            
            *freq = 902300000U + ( 200000U * chIndex);
            *minRate = 0U;
            *maxRate = 3U;
        }
        else if(chIndex < 72U){
            
            *freq = 903000000U + ( 200000U * (chIndex - 64U));
            *minRate = 4U;
            *maxRate = 3U;
        }
        else{
            
            retval = false;
        }
        break;
                    
    case CN_470_510:                    
    
        if(chIndex < 96U){
            
            *freq = 470300000U + ( 200000U * chIndex);
            *minRate = 0U;
            *maxRate = 5U;
            retval = true;
        }
        break;
    }
    
    return retval;
}

uint8_t Region_numChannels(const struct lora_region *self)
{
    return self->numChannels;
}

uint8_t Region_getPayload(const struct lora_region *self, uint8_t rate)
{
    struct lora_data_rate setting;
    uint8_t retval = 0U;
    
    if(Region_getRate(self, rate, &setting)){
        
        retval = setting.payload;
    }
    
    return retval;
}

void Region_getDefaultChannels(const struct lora_region *self, void *receiver, void (*handler)(void *reciever, uint8_t chIndex, uint32_t freq, uint8_t minRate, uint8_t maxRate))
{
    LORA_PEDANTIC(self != NULL)
    LORA_PEDANTIC(handler != NULL)
    
    size_t i;
    
    for(i=0U; i < self->num_default_channels; i++){
        
        struct lora_region_channel ch;
        uint8_t minRate;
        uint8_t maxRate;
        
        (void)memcpy_P(&ch, &self->default_channels[i], sizeof(ch));
        
        (void)getRateRange(self, ch.chIndex, &minRate, &maxRate);
                
        handler(receiver, ch.chIndex, ch.freq * 100U, minRate, maxRate);
    }    
}

uint16_t Region_getOffTimeFactor(const struct lora_region *self, uint8_t band)
{
    LORA_PEDANTIC(self != NULL)
    
    uint16_t retval = 1U;
    
    if(band < self->num_off_time_factors){

        (void)memcpy_P(&retval, &self->off_time_factors[band], sizeof(retval));
    }
    
    return retval;
}

bool Region_validateRate(const struct lora_region *self, uint8_t chIndex, uint8_t minRate, uint8_t maxRate)
{
    LORA_PEDANTIC(self != NULL)
    
    bool retval = false;
    uint8_t min;
    uint8_t max;

    if(getRateRange(self, chIndex, &min, &max)){
        
        if((minRate >= min) && (maxRate <= max)){
            
            retval = true;
        }
    }    
    
    return retval;
}

bool Region_validateFreq(const struct lora_region *self, uint8_t chIndex, uint32_t freq)
{
    return true;
}

bool Region_getRX1DataRate(const struct lora_region *self, uint8_t tx_rate, uint8_t rx1_offset, uint8_t *rx1_rate)
{
    LORA_PEDANTIC(self != NULL)
    LORA_PEDANTIC(rx1_rate != NULL)

    bool retval = false;
    uint16_t index;
    
    if(rx1_offset < self->rx1_row_size){
        
        if(tx_rate < self->rx1_col_size){
        
            index = tx_rate * self->rx1_row_size;
            index += rx1_offset;
            
            (void)memcpy_P(&rx1_rate, &self->rx1_rate[index], sizeof(rx1_rate));
            
            retval = true;
        }    
    }
    
    return retval;
}

bool Region_getRX1Freq(const struct lora_region *self, uint32_t txFreq, uint32_t *freq)
{
    bool retval = false;
    uint8_t chIndex;
    
    switch(self->id){
    default:
    case EU_863_870:
    case EU_433:
    case AS_923:
    case KR_920_923:
    case CN_779_787:
    case CN_470_510:                      
        *freq = txFreq;
        break;                      
    case US_902_928:            
    case AU_915_928:
    
        if(freqToChannel(self, txFreq, &chIndex)){
            
        }        
        break;                
    }
    
    return retval;
}

uint16_t Region_getMaxFCNTGap(const struct lora_region *self)
{
    uint16_t retval = 0U;
    
    (void)memcpy_P(&retval, &self->defaults->max_fcnt_gap, sizeof(retval));
    
    return retval;
}

uint8_t Region_getJA1Delay(const struct lora_region *self)
{
    uint8_t retval;
    memcpy_P(&retval, &self->defaults->ja1_delay, sizeof(retval));
    return retval;
}

/* static functions ***************************************************/

static bool getRateRange(const struct lora_region *self, uint8_t chIndex, uint8_t *minRate, uint8_t *maxRate)
{
    uint8_t i;
    bool retval = false;
    
    for(i=0U; i < self->num_rate_allocation; i++){
        
        struct lora_region_rate_allocation ra;
        
        memcpy_P(&ra, &self->rate_allocation[i], sizeof(ra));
        
        if(chIndex >= ra.minChIndex && chIndex <= ra.maxChIndex){
            
            *minRate = ra.minRate;
            *maxRate = ra.maxRate;
            
            retval = true;
            break;
        }
    }
    
    return retval;
}

static bool freqToChannel(const struct lora_region *self, uint32_t freq, uint8_t *chIndex)
{
    bool retval = false;
    
    switch(self->id){
    default:
    case EU_863_870:
    case EU_433:
    case AS_923:
    case KR_920_923:
    case CN_779_787:  
        break;                      
    case US_902_928:            
    case AU_915_928:
        break;
    }
    
    return retval;    
}

#if !defined(LORA_AVR)
    #undef memcpy_P
#endif
