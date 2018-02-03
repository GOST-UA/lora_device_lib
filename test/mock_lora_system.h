#ifndef MOCK_LORA_SYSTEM_H
#define MOCK_LORA_SYSTEM_H

#include "lora_system.h"

struct mock_channel_config {
    
    uint32_t freq : 24U;    /**< frequency x 10^3 Hz */
    uint8_t minRate : 4U;   /**< minimum allowable data rate */
    uint8_t maxRate : 4U;   /**< maximum allowable data rate */
};

struct mock_system_param {

    uint8_t appEUI[8U];
    uint8_t devEUI[8U];
    
    uint8_t appKey[16U];
    uint8_t appSKey[16U];
    uint8_t nwkSKey[16U];    

    uint32_t devAddr;
    
    struct mock_channel_config chConfig[16U];
    uint8_t chMask[72U / 8U];
    
    uint8_t tx_rate;
    uint8_t tx_power;
    
    uint8_t max_duty_cycle;
    
    uint8_t nb_trans;
    
    uint8_t rx1_dr_offset;    
    uint8_t rx2_data_rate;    
    
    uint8_t rx1_delay;
    
    uint32_t rx2_freq;
    uint8_t rx2_rate;     
    
    uint8_t battery_level;
    
    uint16_t upCounter;
    uint16_t downCounter;    
};

void mock_lora_system_init(struct mock_system_param *self);

#endif
