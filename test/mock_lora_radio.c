#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"

#include "lora_radio.h"

void Radio_init(struct lora_radio *self, const struct lora_board *board)
{
}

uint32_t Radio_resetHardware(struct lora_radio *self)
{
    return 0;
}

void Radio_sleep(struct lora_radio *self)
{
    
}

bool Radio_transmit(struct lora_radio *self, const struct lora_radio_tx_setting *settings, const void *data, uint8_t len)
{
    return true;
}

bool Radio_receive(struct lora_radio *self, const struct lora_radio_rx_setting *settings)
{
    return true;
}

uint8_t Radio_collect(struct lora_radio *self, void *data, uint8_t max)
{
    return 0U;
}

void Radio_interrupt(struct lora_radio *self, uint8_t n, uint64_t time)
{
}

void Radio_setEventHandler(struct lora_radio *self, void *receiver, radioEventCB cb)
{
}

