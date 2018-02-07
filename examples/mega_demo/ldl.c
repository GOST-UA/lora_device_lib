/* Copyright (c) 2018 Cameron Harper
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


#include "lora_mac.h"
#include "lora_radio_sx1272.h"
#include "lora_system.h"

#include <avr/eeprom.h>
#include <avr/io.h>
#include <util/atomic.h>

#include <stdlib.h>

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

bool ready = false;

static uint64_t system_time;

static struct param_store params EEMEM;

volatile struct lora_radio radio;
volatile struct lora_mac mac;

static void setup_external_interrupts(void);
static void response_handler(void *receiver, enum lora_mac_response_type type, const union lora_mac_response_arg *arg);

static uint8_t radio_read(void *receiver);
static void radio_select(void *receiver, bool state);
static void radio_reset(void *receiver, bool state);
static void radio_write(void *receiver, uint8_t data);

/* functions **********************************************************/



void ldl_init(void)
{
    system_time = 0U;
    
    struct lora_board board = {
    
        .receiver = NULL,
        .select = radio_select,
        .reset = radio_reset,
        .write = radio_write,
        .read = radio_read
    };
    
    (void)MAC_init(&mac, NULL, EU_863_870, Radio_init(&radio, &board));
 
    MAC_setResponseHandler(&mac, NULL, response_handler);
}
void ldl_tick(void **state, void (*on_ready)(void **))
{
    if(ready){
        
        if(on_ready != NULL){
            
            on_ready(state);
        }
    }
    
    MAC_tick(&mac);    
}

bool ldl_join(void)
{
    return MAC_join(&mac);
}

bool System_getChannel(void *receiver, uint8_t chIndex, uint32_t *freq, uint8_t *minRate, uint8_t *maxRate)
{
    bool retval = false;
    struct channel_config ch;
    
    if(chIndex < sizeof(params.chConfig)/sizeof(*params.chConfig)){
        
        eeprom_read_block(&ch, &params.chConfig[chIndex], sizeof(ch));
        
        *freq = ch.freq * 100U;
        *minRate = ch.minRate;
        *maxRate = ch.maxRate;
        
        retval = true;
    }
    
    return retval;
}

bool System_setChannel(void *receiver, uint8_t chIndex, uint32_t freq, uint8_t minRate, uint8_t maxRate)
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

bool System_maskChannel(void *receiver, uint8_t chIndex)
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

bool System_unmaskChannel(void *receiver, uint8_t chIndex)
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

bool System_channelIsMasked(void *receiver, uint8_t chIndex)
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

uint8_t System_getRX1DROffset(void *receiver)
{
    return eeprom_read_byte(&params.rx1_dr_offset);
}

void System_setRX1DROffset(void *receiver, uint8_t value)
{
    eeprom_update_byte(&params.rx1_dr_offset, value);
}

uint8_t System_getRX2DataRate(void *receiver)
{
    return eeprom_read_byte(&params.rx2_data_rate);
}

void System_setRX2DataRate(void *receiver, uint8_t value)
{
    eeprom_update_byte(&params.rx2_data_rate, value);
}

uint8_t System_getMaxDutyCycle(void *receiver)
{
    return eeprom_read_byte(&params.max_duty_cycle);
}

void System_setMaxDutyCycle(void *receiver, uint8_t value)
{
    eeprom_update_byte(&params.max_duty_cycle, value);
}

uint8_t System_getRX1Delay(void *receiver)
{
    return eeprom_read_byte(&params.rx1_delay);
}

void System_setRX1Delay(void *receiver, uint8_t value)
{
    eeprom_update_byte(&params.rx1_delay, value);
}

uint8_t System_getNbTrans(void *receiver)
{
    return eeprom_read_byte(&params.nb_trans);
}

uint8_t System_getTXPower(void *receiver)
{
    return eeprom_read_byte(&params.tx_power);
}

void System_setTXPower(void *receiver, uint8_t value)
{
    eeprom_update_byte(&params.tx_power, value);
}

uint8_t System_getTXRate(void *receiver)
{
    return eeprom_read_byte(&params.tx_rate);
}

void System_setTXRate(void *receiver, uint8_t value)
{
    eeprom_update_byte(&params.tx_rate, value);
}

void System_getAppEUI(void *receiver, void *eui)
{
    eeprom_read_block(eui, &params.appEUI, sizeof(params.appEUI));
}

void System_getDevEUI(void *receiver, void *eui)
{
    eeprom_read_block(eui, &params.devEUI, sizeof(params.devEUI));
}

void System_getAppKey(void *receiver, void *key)
{
    eeprom_read_block(key, &params.appKey, sizeof(params.appKey));
}

void System_getNwkSKey(void *receiver, void *key)
{
    eeprom_read_block(key, &params.nwkSKey, sizeof(params.nwkSKey));
}

void System_setNwkSKey(void *receiver, const void *key)
{
    eeprom_write_block(key, &params.nwkSKey, sizeof(params.nwkSKey));
}

void System_getAppSKey(void *receiver, void *key)
{
    eeprom_read_block(key, &params.appSKey, sizeof(params.appSKey));
}

void System_setAppSKey(void *receiver, const void *key)
{
    eeprom_write_block(key, &params.appSKey, sizeof(params.appSKey));
}

uint32_t System_getDevAddr(void *receiver)
{
    return eeprom_read_dword(&params.devAddr);
}

void System_setDevAddr(void *receiver, uint32_t devAddr)
{
    eeprom_write_dword(&params.devAddr, devAddr);
}

uint16_t System_getDown(void *receiver)
{
    return 0U;
}

void System_resetUp(void *receiver)
{
    eeprom_write_word(&upCounter, 0U);
}

void System_resetDown(void *receiver)
{
    eeprom_write_word(&downCounter, 0U);
}

uint16_t System_incrementUp(void *receiver)
{
    uint16_t value = eeprom_read_word(&upCounter);    
    eeprom_update_word(&upCounter, value + 1U);
    return value;
}

bool System_receiveDown(void *receiver, uint16_t counter, uint16_t maxGap)
{
    bool retval = false;
    uint16_t value = eeprom_read_word(&downCounter);
    
    if((uint32_t)counter < ((uint32_t)value + (uint32_t)maxGap)){
        
        eeprom_write_word(&downCounter, counter);
        retval = true;
    }
    
    return retval;
}

void System_setLinkStatus(void *receiver, uint8_t margin, uint8_t gwCount)
{
}

uint32_t System_getRX2Freq(void *receiver)
{
    return eeprom_read_dword(&params.rx2_freq);
}

void System_setRX2Freq(void *receiver, uint32_t freq)
{
    eeprom_update_dword(&params.rx2_freq, freq);
}

uint8_t System_getRX2Rate(void *receiver)
{
    return eeprom_read_byte(&params.rx2_rate);
}

ISR(INT0_vect){
    
    Radio_interrupt(&radio, 0U, System_time());    
}

ISR(INT1_vect){
    
    Radio_interrupt(&radio, 1U, System_time());
}

uint64_t System_time(void)
{
    uint64_t retval;
    
    ATOMIC_BLOCK(ATOMIC_FORCEON)
    {
        retval = system_time;
    }
    
    return retval;
}

void System_sleep(uint32_t interval)
{
}

void System_atomic_setPtr(void **receiver, void *value)
{
    ATOMIC_BLOCK(ATOMIC_FORCEON)
    {
        *receiver = value;
    }    
}

uint8_t System_rand(void)
{
    return (uint8_t)rand();
}

uint8_t System_getBatteryLevel(void *receiver)
{
    return 255U;    // not available
}


/* static functions ***************************************************/

static void response_handler(void *receiver, enum lora_mac_response_type type, const union lora_mac_response_arg *arg)
{
    switch(type){
    case LORA_MAC_READY:
    case LORA_MAC_DATA_TIMEOUT:
    case LORA_MAC_RX:
    case LORA_MAC_JOIN_SUCCESS:
    case LORA_MAC_JOIN_TIMEOUT:
    default:
        break;
    }        
}

static void radio_select(void *receiver, bool state)
{
}

static void radio_reset(void *receiver, bool state)
{        
}

static void radio_write(void *receiver, uint8_t data)
{
    SPDR = data;
    while(!(SPSR & (1U << SPIF)));
}

static uint8_t radio_read(void *receiver)
{
    radio_write(NULL, 0U);
    return SPDR;
}

static void enable_spi(void)
{    
    DDRB |= (1U<<DDB2) | (1U<<DDB1) | (1U<<DDB0);
    
    /* enable | master */
    SPCR = (1U<<SPE) | (1U<<MSTR);
        
#if F_CPU <= 16000000U    
    
    /* double speed SPI (F_CPU / 2) */
    SPSR |= (1U<<SPI2X);
    
#endif
}

static void setup_external_interrupts(void)
{
    /* configure INT0 and INT1 for rising edge interrupt */
    EICRA |= (1U<<ISC11) | (1U<<ISC10) | (1U<<ISC01) | (1U<<ISC00);
    
    /* unmask INT0 INT1 */
    PCMSK0 |= (1U<<INT1) | (1U<<INT0);
    
    /* clear flags? */
    EIFR |= (1U<<INTF1) | (1U<<INTF0);
}

