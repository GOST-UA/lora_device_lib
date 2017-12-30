#include "lora_system.h"

#include <avr/eeprom.h>

struct channel_config {
    
    uint32_t freq : 24U;    /**< frequency x 10^3 Hz */
    uint8_t minRate : 4U;   /**< minimum allowable data rate */
    uint8_t maxRate : 4U;   /**< maximum allowable data rate */
};

struct param_store {

    uint8_t appEUI[8U];
    uint8_t devEUI[8U];
    
    uint8_t appKey[16U];
    uint8_t appSKey[16U];
    uint8_t nwkSKey[16U];    

    uint32_t devAddr;
    
    struct channel_config chConfig[16U];
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
    
    uint16_t crc;
};

/*todo wear level*/
uint16_t upCounter EEMEM;
uint16_t downCounter EEMEM;

static struct param_store params EEMEM;

bool System_getChannel(void *owner, uint8_t chIndex, uint32_t *freq, uint8_t *minRate, uint8_t *maxRate)
{
    bool retval = false;
    struct channel_config ch;
    
    if(chIndex < sizeof(params.chConfig)/sizeof(*params.chConfig)){
        
        eeprom_read_block(&ch, &params.chConfig[chIndex], sizeof(ch));
        
        *freq = ch.freq;
        *minRate = ch.minRate;
        *maxRate = ch.maxRate;
        
        retval = true;
    }
    
    return retval;
}

bool System_setChannel(void *owner, uint8_t chIndex, uint32_t freq, uint8_t minRate, uint8_t maxRate)
{
    bool retval = false;
    struct channel_config ch;
    
    if(chIndex < sizeof(params.chConfig)/sizeof(*params.chConfig)){
        
        ch.freq = freq / 100U;
        ch.minRate = minRate & 0xfU;
        ch.maxRate = maxRate & 0xfU;
        
        eeprom_update_block(&params.chConfig[chIndex], &ch, sizeof(ch));
        
        retval = true;
    }
    
    return retval;
}

bool System_maskChannel(void *owner, uint8_t chIndex)
{
    bool retval = false;
    uint8_t c;
    uint8_t i;
    uint8_t mask;
    
    if(chIndex < (sizeof(params.chMask)*8U)){
    
        i = chIndex / 8U;
        mask = 1U << (chIndex % 8U);
        
        c = eeprom_read_byte(&params.chMask[i]);
        
        c |= mask;
        
        eeprom_write_byte(&params.chMask[i], c);
        
        retval = true;
    }
    
    return retval;
}

bool System_unmaskChannel(void *owner, uint8_t chIndex)
{
    bool retval = false;
    uint8_t c;
    uint8_t i;
    uint8_t mask;
    
    if(chIndex < (sizeof(params.chMask)*8U)){
    
        i = chIndex / 8U;
        mask = 1U << (chIndex % 8U);
        
        c = eeprom_read_byte(&params.chMask[i]);
        
        c &= ~(mask);
        
        eeprom_write_byte(&params.chMask[i], c);
        
        retval = true;
    }
    
    return retval;
}

bool System_channelIsMasked(void *owner, uint8_t chIndex)
{
    bool retval = false;
    uint8_t c;
    uint8_t i;
    uint8_t mask;
    
    if(chIndex < (sizeof(params.chMask)*8U)){
    
        i = chIndex / 8U;
        mask = 1U << (chIndex % 8U);
        
        c = eeprom_read_byte(&params.chMask[i]);
        
        retval = ((c & mask) == mask);
    }
    
    return retval;    
}

uint8_t System_getRX1DROffset(void *owner)
{
    return eeprom_read_byte(&params.rx1_dr_offset);
}

void System_setRX1DROffset(void *owner, uint8_t value)
{
    eeprom_update_byte(&params.rx1_dr_offset, value);
}

uint8_t System_getRX2DataRate(void *owner)
{
    return eeprom_read_byte(&params.rx2_data_rate);
}

void System_setRX2DataRate(void *owner, uint8_t value)
{
    eeprom_update_byte(&params.rx2_data_rate, value);
}

uint8_t System_getMaxDutyCycle(void *owner)
{
    return eeprom_read_byte(&params.max_duty_cycle);
}

void System_setMaxDutyCycle(void *owner, uint8_t value)
{
    eeprom_update_byte(&params.max_duty_cycle, value);
}

uint8_t System_getRX1Delay(void *owner)
{
    return eeprom_read_byte(&params.rx1_delay);
}

void System_setRX1Delay(void *owner, uint8_t value)
{
    eeprom_update_byte(&params.rx1_delay, value);
}

uint8_t System_getNbTrans(void *owner)
{
    return eeprom_read_byte(&params.nb_trans);
}

uint8_t System_getTXPower(void *owner)
{
    return eeprom_read_byte(&params.tx_power);
}

uint8_t System_getTXRate(void *owner)
{
    return eeprom_read_byte(&params.tx_rate);
}

void System_getAppEUI(void *owner, uint8_t *eui)
{
    eeprom_read_block(eui, &params.appEUI, sizeof(params.appEUI));
}

void System_getDevEUI(void *owner, uint8_t *eui)
{
    eeprom_read_block(eui, &params.devEUI, sizeof(params.devEUI));
}

void System_getAppKey(void *owner, uint8_t *key)
{
    eeprom_read_block(key, &params.appKey, sizeof(params.appKey));
}

void System_getNwkSKey(void *owner, uint8_t *key)
{
    eeprom_read_block(key, &params.nwkSKey, sizeof(params.nwkSKey));
}

void System_setNwkSKey(void *owner, const uint8_t *key)
{
    eeprom_write_block(key, &params.nwkSKey, sizeof(params.nwkSKey));
}

void System_getAppSKey(void *owner, uint8_t *key)
{
    eeprom_read_block(key, &params.appSKey, sizeof(params.appSKey));
}

void System_setAppSKey(void *owner, const uint8_t *key)
{
    eeprom_write_block(key, &params.appSKey, sizeof(params.appSKey));
}

uint32_t System_getDevAddr(void *owner)
{
    return eeprom_read_dword(&params.devAddr);
}

void System_setDevAddr(void *owner, uint32_t devAddr)
{
    eeprom_write_dword(&params.devAddr, devAddr);
}

uint16_t System_getDown(void *owner)
{
    return 0U;
}

void System_resetUp(void *owner)
{
    eeprom_write_word(&upCounter, 0U);
}

void System_resetDown(void *owner)
{
    eeprom_write_word(&downCounter, 0U);
}

uint16_t System_incrementUp(void *owner)
{
    uint16_t value = eeprom_read_word(&upCounter);    
    eeprom_update_word(&upCounter, value + 1U);
    return value;
}

bool System_receiveDown(void *owner, uint16_t counter, uint16_t maxGap)
{
    bool retval = false;
    uint16_t value = eeprom_read_word(&downCounter);
    
    if((uint32_t)counter < ((uint32_t)value + (uint32_t)maxGap)){
        
        eeprom_write_word(&downCounter, counter);
    }
    
    return retval;
}

void System_logLinkStatus(void *owner, uint8_t margin, uint8_t gwCount)
{
}

uint32_t System_getRX2Freq(void *owner)
{
    return eeprom_read_dword(&params.rx2_freq);
}

void System_setRX2Freq(void *owner, uint32_t freq)
{
    eeprom_update_dword(&params.rx2_freq, freq);
}

uint8_t System_getRX2Rate(void *owner)
{
    return eeprom_read_byte(&params.rx2_rate);
}

/* static functions ***************************************************/

