#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdlib.h>

#include "cmocka.h"

#include "lora_system.h"
#include "mock_lora_system.h"

#include <string.h>

void System_usleep(uint32_t interval)
{    
}

void System_atomic_setPtr(void **receiver, void *value)
{
    *receiver = value;
}

bool System_getChannel(void *receiver, uint8_t chIndex, uint32_t *freq, uint8_t *minRate, uint8_t *maxRate)
{
    bool retval = false;
    struct mock_system_param *self = (struct mock_system_param *)receiver;    
    
    if(chIndex < sizeof(self->chConfig)/sizeof(*self->chConfig)){
        
        *freq = self->chConfig[chIndex].freq * 100U;
        *minRate = self->chConfig[chIndex].minRate;
        *maxRate = self->chConfig[chIndex].maxRate;
        
        retval = true;
    }
    
    return retval;
}

bool System_setChannel(void *receiver, uint8_t chIndex, uint32_t freq, uint8_t minRate, uint8_t maxRate)
{
    bool retval = false;
    struct mock_system_param *self = (struct mock_system_param *)receiver;    
    
    if(chIndex < sizeof(self->chConfig)/sizeof(*self->chConfig)){
    
        self->chConfig[chIndex].freq = freq / 100U;
        self->chConfig[chIndex].minRate = minRate & 0xfU;
        self->chConfig[chIndex].maxRate = maxRate & 0xfU;
        
        retval = true;
    }
    
    return retval;
}

bool System_maskChannel(void *receiver, uint8_t chIndex)
{
    bool retval = false;
    uint8_t c;
    uint8_t i;
    uint8_t mask;
    struct mock_system_param *self = (struct mock_system_param *)receiver;    
    
    if(chIndex < (sizeof(self->chMask)*8U)){
    
        i = chIndex / 8U;
        mask = 1U << (chIndex % 8U);
        
        c = self->chMask[i];
        
        c |= mask;
        
        self->chMask[i] = c;
        
        retval = true;
    }
    
    return retval;
}

bool System_unmaskChannel(void *receiver, uint8_t chIndex)
{
    bool retval = false;
    uint8_t c;
    uint8_t i;
    uint8_t mask;
    struct mock_system_param *self = (struct mock_system_param *)receiver;    
    
    if(chIndex < (sizeof(self->chMask)*8U)){
    
        i = chIndex / 8U;
        mask = 1U << (chIndex % 8U);
        
        c = self->chMask[i];
        
        c &= ~(mask);
        
        self->chMask[i] = c;
        
        retval = true;
    }
    
    return retval;
}

bool System_channelIsMasked(void *receiver, uint8_t chIndex)
{
    bool retval = false;
    uint8_t c;
    uint8_t i;
    uint8_t mask;
    struct mock_system_param *self = (struct mock_system_param *)receiver;    
    
    if(chIndex < (sizeof(self->chMask)*8U)){
    
        i = chIndex / 8U;
        mask = 1U << (chIndex % 8U);
        
        c = self->chMask[i];
        
        retval = ((c & mask) == mask);
    }
    
    return retval;    
}

uint8_t System_getRX1DROffset(void *receiver)
{
    struct mock_system_param *self = (struct mock_system_param *)receiver;    
    return self->rx1_dr_offset;    
}

void System_setRX1DROffset(void *receiver, uint8_t value)
{
    struct mock_system_param *self = (struct mock_system_param *)receiver;    
    self->rx1_dr_offset = value;
}

uint8_t System_getRX2DataRate(void *receiver)
{
    struct mock_system_param *self = (struct mock_system_param *)receiver;    
    return self->rx2_data_rate;    
}

void System_setRX2DataRate(void *receiver, uint8_t value)
{
    struct mock_system_param *self = (struct mock_system_param *)receiver;    
    self->rx2_data_rate = value;
}

uint8_t System_getMaxDutyCycle(void *receiver)
{
    struct mock_system_param *self = (struct mock_system_param *)receiver;    
    return self->max_duty_cycle;    
}

void System_setMaxDutyCycle(void *receiver, uint8_t value)
{
    struct mock_system_param *self = (struct mock_system_param *)receiver;    
    self->max_duty_cycle = value;
}

uint8_t System_getRX1Delay(void *receiver)
{
    struct mock_system_param *self = (struct mock_system_param *)receiver;    
    return self->rx1_delay;    
}

void System_setRX1Delay(void *receiver, uint8_t value)
{
    struct mock_system_param *self = (struct mock_system_param *)receiver;    
    self->rx1_delay = value;
}

uint8_t System_getNbTrans(void *receiver)
{
    struct mock_system_param *self = (struct mock_system_param *)receiver;    
    return self->nb_trans;    
}

uint8_t System_getTXPower(void *receiver)
{
    struct mock_system_param *self = (struct mock_system_param *)receiver;    
    return self->tx_power;    
}

void System_setTXPower(void *receiver, uint8_t value)
{
    struct mock_system_param *self = (struct mock_system_param *)receiver;    
    self->tx_power = value;
}

uint8_t System_getTXRate(void *receiver)
{
    struct mock_system_param *self = (struct mock_system_param *)receiver;    
    return self->tx_rate;    
}

void System_setTXRate(void *receiver, uint8_t value)
{
    struct mock_system_param *self = (struct mock_system_param *)receiver;    
    self->tx_rate = value;    
}

void System_getAppEUI(void *receiver, void *eui)
{
    struct mock_system_param *self = (struct mock_system_param *)receiver;    
    (void)memcpy(eui, self->appEUI, sizeof(self->appEUI));    
}

void System_getDevEUI(void *receiver, void *eui)
{
    struct mock_system_param *self = (struct mock_system_param *)receiver;    
    (void)memcpy(eui, self->devEUI, sizeof(self->devEUI));    
}

void System_getAppKey(void *receiver, void *key)
{
    struct mock_system_param *self = (struct mock_system_param *)receiver;    
    (void)memcpy(key, self->appKey, sizeof(self->appKey));
}

void System_getNwkSKey(void *receiver, void *key)
{
    struct mock_system_param *self = (struct mock_system_param *)receiver;    
    (void)memcpy(key, self->nwkSKey, sizeof(self->nwkSKey));
}

void System_setNwkSKey(void *receiver, const void *key)
{
    struct mock_system_param *self = (struct mock_system_param *)receiver;    
    (void)memcpy(self->nwkSKey, key, sizeof(self->nwkSKey));
}

void System_getAppSKey(void *receiver, void *key)
{
    struct mock_system_param *self = (struct mock_system_param *)receiver;    
    (void)memcpy(key, self->appSKey, sizeof(self->appSKey));
}

void System_setAppSKey(void *receiver, const void *key)
{
    struct mock_system_param *self = (struct mock_system_param *)receiver;    
    (void)memcpy(self->appSKey, key, sizeof(self->appSKey));
}

uint32_t System_getDevAddr(void *receiver)
{
    struct mock_system_param *self = (struct mock_system_param *)receiver;    
    return self->devAddr;    
}

void System_setDevAddr(void *receiver, uint32_t devAddr)
{
    struct mock_system_param *self = (struct mock_system_param *)receiver;    
    self->devAddr = devAddr;    
}

uint16_t System_getDown(void *receiver)
{
    struct mock_system_param *self = (struct mock_system_param *)receiver;    
    return self->downCounter;    
}

void System_resetUp(void *receiver)
{
    struct mock_system_param *self = (struct mock_system_param *)receiver;    
    self->upCounter = 0U;   
}

void System_resetDown(void *receiver)
{
    struct mock_system_param *self = (struct mock_system_param *)receiver;    
    self->downCounter = 0U;   
}

uint16_t System_incrementUp(void *receiver)
{
    struct mock_system_param *self = (struct mock_system_param *)receiver;    
    self->upCounter++;   
    return (self->upCounter - 1U);       
}

bool System_receiveDown(void *receiver, uint16_t counter, uint16_t maxGap)
{
    struct mock_system_param *self = (struct mock_system_param *)receiver;    
    bool retval = false;
    
    if((uint32_t)counter < ((uint32_t)self->downCounter + (uint32_t)maxGap)){
        
        self->downCounter = counter;
        retval = true;
    }
    
    return retval;
}

void System_logLinkStatus(void *receiver, uint8_t margin, uint8_t gwCount)
{
}

uint32_t System_getRX2Freq(void *receiver)
{
    struct mock_system_param *self = (struct mock_system_param *)receiver;    
    return self->rx2_freq;
}

void System_setRX2Freq(void *receiver, uint32_t freq)
{
    struct mock_system_param *self = (struct mock_system_param *)receiver;    
    self->rx2_freq = freq;
}

uint8_t System_getRX2Rate(void *receiver)
{
    struct mock_system_param *self = (struct mock_system_param *)receiver;    
    return self->rx2_rate;    
}

uint8_t System_rand(void)
{
    return (uint8_t)rand();
}

uint8_t System_getBatteryLevel(void *receiver)
{
    struct mock_system_param *self = (struct mock_system_param *)receiver;    
    return self->battery_level;    
}

void mock_lora_system_init(struct mock_system_param *self)
{
    (void)memset(self, 0, sizeof(*self));
}


