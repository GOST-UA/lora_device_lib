#ifndef LORA_REGION_H
#define LORA_REGION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lora_radio_defs.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

enum lora_region_id {
    EU_863_870,
    US_902_928,
    CN_779_787,
    EU_433,
    AUSTRALIA_915_928,
    CN_470_510,
    AS_923,
    KR_920_923
};

/** LoRa Data Rate is a function of spreading factor and bandwidth
 *
 * */
struct data_rate {
    enum spreading_factor sf;   ///< spreading factor
    enum signal_bandwidth bw;   ///< bandwidth
    uint16_t payload;           ///< maximum payload (accounts for max dwell time)
};

struct channel_info {

    uint32_t freq;          // carrier frequency
    const uint8_t *rates;   // permitted data rates
    size_t num_rates;
};

struct region_defaults {

    uint16_t max_fcnt_gap;          // counter values  
    uint8_t receive_delay1;         // seconds
    uint8_t receive_delay2;         // seconds
    uint8_t join_accept_delay1;     // seconds
    uint8_t join_accept_delay2;     // seconds
    uint8_t adr_ack_limit;          // seconds
    uint8_t adr_ack_delay;          // seconds
    uint8_t ack_timeout;            // seconds
    uint8_t ack_dither;             // seconds (+/- around ack_timeout)
};

/** a range of frequencies ( begin .. end ) */
struct region_band {    
    uint32_t begin; ///< Hz
    uint32_t end;   ///< Hz
    uint8_t id;     ///< several bands may be combined into one by id
};

struct region_info {

    const struct data_rate **rates;
    size_t num_rates;
    
    const uint8_t *slot1_rate;
    size_t num_uprates;
    size_t num_downrates;
    
    uint8_t slot2_rate;
    uint32_t slot2_frequency;

    const struct region_band *bands;    /**< collection of frequency band definitions which map to "duty cycle bands" */
    size_t num_bands;

    const uint16_t *off_time_factors;   /**< collection of off-time factors indexed per "duty cycle band" */
    size_t num_off_time_factors;
    
    const struct channel_info *default_channels;
    size_t num_default_channels;

    const struct region_defaults *defaults;
};


/** Retrieve datarate parameters for a given DR integer and region
 *
 * @param[in] region
 * @param[in] rate DRn integer (e.g. 0 == DR0)
 *
 * @return data rate parameter structure
 *
 * @retval NULL data rate not recognised 
 *
 * */
const struct data_rate *LoraRegion_getDataRateParameters(enum lora_region_id region, uint8_t rate);

/** Compile time settings and/or incomplete implementation means it is
 * possible not all regions are recognised.
 * 
 * @param[in] region
 * @return true if this region is recognised
 *
 * */
bool LoraRegion_regionSupported(enum lora_region_id region);

bool LoraRegion_getDefaultDataRate(enum lora_region_id region, uint32_t frequency, uint8_t *rate);

/** Test that frequency is acceptable for region
 *
 * @note will also return the frequency band index useful for determining
 *       duty cycle constraints
 *
 * @param[in] region
 * @param[in] frequency
 * @param[out] band     sub-band
 *
 * @return true if frequency within bounds
 *
 * */
bool LoraRegion_validateFrequency(enum lora_region_id region, uint32_t frequency, uint8_t *band);

uint16_t LoraRegion_getDutyCycle(enum lora_region_id region, uint8_t band);

uint8_t LoraRegion_numberOfBands(enum lora_region_id region);

/** get acceptible power settings for a channel */
bool LoraRegion_getPowerForChannel(enum lora_region_id region, uint8_t *power, uint8_t maxPower); 

bool LoraRegion_getOffTimeFactor(enum lora_region_id region, uint8_t band, uint16_t *offTimeFactor);

const struct region_defaults *LoraRegion_getDefaultSettings(enum lora_region_id region);


#ifdef __cplusplus
}
#endif

#endif
