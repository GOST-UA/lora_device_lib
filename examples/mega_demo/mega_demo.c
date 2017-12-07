#include "lora_device_lib.h"
#include "lora_radio_sx1272.h"

#include <string.h>

#include <avr/interrupt.h>
#include <util/atomic.h>
#include <avr/io.h>
#include <avr/wdt.h>

#define BAUD 9600U
#define F_CPU 8000000U

static void responseHandler(void *receiver, enum lora_mac_response_type type, const union lora_mac_response_arg *arg);

void main(void) __attribute__((noreturn));

volatile struct lora_radio radio;
volatile struct lora_device_lib ldl;

volatile bool do_tick;

/* note this will be called via LDL_tick() */
static void responseHandler(void *receiver, enum lora_mac_response_type type, const union lora_mac_response_arg *arg)
{
}

static void radioSelect(void *receiver, bool state)
{
}

static void radioReset(void *receiver, bool state)
{
}

static void radioWrite(void *receiver, uint8_t data)
{
}

static uint8_t radioRead(void *receiver)
{
    return 0;
}

static void setup_spi(void)
{
    DDRB = ((1<<DDB2)|(1<<DDB1)|(1<<DDB0));
    SPCR = ((1<<SPE)|(1<<MSTR)|(1<<SPR0)|(1<<CPOL)|(1<<CPHA));
}

static void setup_io(void)
{
    
}

static void setup_uart(void)
{
    UBRR0L = (uint8_t)(F_CPU/(BAUD*16L)-1);
    UCSR0B = (1<<RXEN0) | (1<<TXEN0) | (1<<RXCIE0) | (1<<TXCIE0);
    UCSR0C = (1<<UCSZ00) | (1<<UCSZ01);
}

static void setup_lora(void)
{
    /* setup LDL */    
    struct lora_board board = {
    
        .receiver = NULL,
        .select = radioSelect,
        .reset = radioReset,
        .write = radioWrite,
        .read = radioRead
    };
    
    (void)LDL_init(&ldl, EU_863_870, Radio_init(&radio, &board));
 
    LDL_setResponseHandler(&ldl, NULL, responseHandler);
}

void main(void)
{
    setup_io();
    setup_uart();    
    setup_spi();
    setup_lora();
 
    const char message[] = "dummy";
 
    (void)LDL_send(&ldl, false, 1, message, sizeof(message));
    
    while(true){
    
        LDL_tick(&ldl);                    
    }
}

#if 0

ISR(Ticker){
    
    do_tick = true;
}

ISR(D0){
    
    Radio_interrupt(&radio, 0);
    do_tick = true;
}

ISR(D1){
    
    Radio_interrupt(&radio, 1);
    do_tick = true;
}

ISR(D2){
    
    Radio_interrupt(&radio, 2);
    do_tick = true;
}

ISR(D4){
    
    Radio_interrupt(&radio, 3);
    do_tick = true;
}

#endif


uint64_t System_getTime(void)
{
    return 0U;
}

void System_usleep(uint32_t interval)
{
}

void System_atomic_setPtr(void **receiver, void *value)
{
    *receiver = value;
}

void System_rand(uint8_t *data, size_t len)
{
    (void)memset(data, 0, len);
}

void System_getAppEUI(void *owner, uint8_t *eui)
{
    (void)memset(eui, 0, 8U);
}

void System_getDevEUI(void *owner, uint8_t *eui)
{
    (void)memset(eui, 0, 8U);
}

void System_getAppKey(void *owner, uint8_t *key)
{
    (void)memset(key, 0, 16U);
}
