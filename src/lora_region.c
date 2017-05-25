#include "lora_region.h"

static const struct data_rate dr0_EU_863_870 = {
    .sf = SF_12,
    .bw = BW_125,
    .payload = 59U,
};

static const struct data_rate dr1_EU_863_870 = {
    .sf = SF_11,
    .bw = BW_125,
    .payload = 59U,
};
    
static const struct data_rate dr2_EU_863_870 = {
    .sf = SF_10,
    .bw = BW_125,
    .payload = 59U,        
};
    
static const struct data_rate dr3_EU_863_870 = {
    .sf = SF_9,
    .bw = BW_125,
    .payload = 123U,        
};
    
static const struct data_rate dr4_EU_863_870 = {
    .sf = SF_8,
    .bw = BW_125,
    .payload = 230U,        
};

static const struct data_rate dr5_EU_863_870 = {
    .sf = SF_7,
    .bw = BW_125,
    .payload = 230U,        
};
    
static const struct data_rate dr6_EU_863_870 = {
    .sf = SF_7,
    .bw = BW_250,
    .payload = 230U,        
};

static const struct data_rate *rates_EU_863_870[] = {
    &dr0_EU_863_870,
    &dr1_EU_863_870,
    &dr2_EU_863_870,
    &dr3_EU_863_870,
    &dr4_EU_863_870,
    &dr5_EU_863_870,
    &dr6_EU_863_870,
};

static const uint8_t erp_EU_863_870[] = {
    20,
    14,
    11,
    8,
    5,
    2
};

static const uint16_t off_time_factors_EU_863_870[] = {
    100,    // 1.0% (band 0)
    100,    // 1.0% (band 1)    
    1000,   // 0.1% (band 2)
    10,     // 10.0% (band 3)
    100,    // 1.0% (band 4)
};

static const struct region_band bands_EU_863_870[] = {
    {
        .begin = 863000000U,
        .end = 865000000U,
        .id = 2
    },
    {
        .begin = 865000000,
        .end = 868000000,
        .id = 0
    },
    {
        .begin = 868000000,
        .end = 868600000,
        .id = 1
    },
    {
        .begin = 868700000,
        .end = 869200000,
        .id = 2
    },
    {
        .begin = 868700000,
        .end = 869200000,
        .id = 3
    },
    {
        .begin = 868700000,
        .end = 869200000,
        .id = 4
    }
};

static const uint8_t default_channel_rates_EU_863_870[] = {
    0,  // DR0
    1,  // DR1
    2,  // DR2
    3,  // DR3
    4,  // DR4
    5,  // DR5
    6,  // DR6
    7   // DR7
};

static const uint8_t slot1_rate_EU_863_870[] = {
    0, 0, 0, 0, 0, 0,   // DR0 upstream
    1, 0, 0, 0, 0, 0,   // DR1 upstream
    2, 1, 0, 0, 0, 0,   // DR2 upstream
    3, 2, 1, 0, 0, 0,   // DR3 upstream
    4, 3, 2, 1, 0, 0,   // DR4 upstream
    5, 4, 3, 2, 1, 0,   // DR5 upstream
    6, 5, 4, 3, 2, 1,   // DR6 upstream
    7, 6, 5, 4, 3, 2,   // DR7 upstream 
};

static const struct channel_info default_channels_EU_863_870[] = {
    {
        .freq = 868100000U,
        .rates = default_channel_rates_EU_863_870,
        .num_rates = sizeof(default_channel_rates_EU_863_870)/sizeof(*default_channel_rates_EU_863_870)
    },
    {
        .freq = 868300000U,
        .rates = default_channel_rates_EU_863_870,
        .num_rates = sizeof(default_channel_rates_EU_863_870)/sizeof(*default_channel_rates_EU_863_870)
    },
    {
        .freq = 868500000U,
        .rates = default_channel_rates_EU_863_870,
        .num_rates = sizeof(default_channel_rates_EU_863_870)/sizeof(*default_channel_rates_EU_863_870)
    }
};

static const struct region_defaults defaults_EU_863_870 = {
    .max_fcnt_gap = 16384U,
    .receive_delay1 = 1U,
    .receive_delay2 = 2U,
    .join_accept_delay1 = 5U,
    .join_accept_delay2 = 6U,
    .adr_ack_delay = 32,
    .adr_ack_limit = 64,
    .ack_timeout = 2,
    .ack_dither = 1
};

static const struct region_info info_EU_863_870 = {

    .rates = rates_EU_863_870,
    .num_rates = sizeof(rates_EU_863_870)/sizeof(*rates_EU_863_870),
    
    .slot1_rate = slot1_rate_EU_863_870,
    .num_uprates =  6U,
    .num_downrates = 6U,
    
    .slot2_rate = 0U,
    .slot2_frequency = 869525000U,
    
    .default_channels = default_channels_EU_863_870,
    .num_default_channels = sizeof(default_channels_EU_863_870)/sizeof(*default_channels_EU_863_870),

    .defaults = &defaults_EU_863_870,

    .bands = bands_EU_863_870,
    .num_bands = sizeof(bands_EU_863_870)/sizeof(*bands_EU_863_870),

    .off_time_factors = off_time_factors_EU_863_870,
    .num_off_time_factors = sizeof(off_time_factors_EU_863_870) / sizeof(*off_time_factors_EU_863_870),    
};

/**********************************************************************/


static const struct region_info *regions[] = {
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

const struct data_rate *LoraRegion_getDataRateParameters(enum lora_region_id region, uint8_t rate)
{
    const struct data_rate *retval = NULL;

    if(regions[region] != NULL){

        if(rate <= regions[region]->num_rates){

            retval = regions[region]->rates[rate];
        }
    }

    return retval;
}

bool LoraRegion_regionSupported(enum lora_region_id region)
{
    return (regions[region] != NULL);
}

bool LoraRegion_validateFrequency(enum lora_region_id region, uint32_t frequency, uint8_t *band)
{
    bool retval = false;
    size_t i;

    if(regions[region] != NULL){

        for(i=0U; i < regions[region]->num_bands; i++){

            if((frequency >= regions[region]->bands[i].begin) && (frequency <= regions[region]->bands[i].end)){

                retval = true;
                *band = regions[region]->bands[i].id;
                break;
            }
        }
    }

    return retval;
}

bool LoraRegion_validateRate(enum lora_region_id region, uint32_t frequency, uint8_t rate)
{
    bool retval = false;
    uint8_t band;

    if(LoraRegion_validateFrequency(region, frequency, &band)){

        retval = true;
    }

    return retval;
}

bool LoraRegion_getOffTimeFactor(enum lora_region_id region, uint8_t band, uint16_t *offTimeFactor)
{
    bool retval = false;

    if(regions[region] != NULL){

        if(band < regions[region]->num_off_time_factors){

            *offTimeFactor = regions[region]->off_time_factors[band];
            retval = true;
        }
    }

    return retval;
}

const struct region_defaults *LoraRegion_getDefaultSettings(enum lora_region_id region)
{
    const struct region_defaults *retval = NULL;

    if(regions[region] != NULL){

        retval = regions[region]->defaults;
    }

    return retval;    
}

