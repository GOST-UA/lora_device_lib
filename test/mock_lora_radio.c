#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <string.h>

#include "cmocka.h"

#include "lora_radio.h"

struct lora_radio * Radio_init(struct lora_radio *self, const struct lora_board *board)
{
    return mock_ptr_type(struct lora_radio *);
}

void Radio_reset(struct lora_radio *self, bool state)
{
}

void Radio_sleep(struct lora_radio *self)
{    
}

bool Radio_transmit(struct lora_radio *self, const struct lora_radio_tx_setting *settings, const void *data, uint8_t len)
{
    return mock_type(bool);
}

bool Radio_receive(struct lora_radio *self, const struct lora_radio_rx_setting *settings)
{
    return mock_type(bool);
}

/* to setup:
 * 
 * will_return( <size of data> )
 * will_return( <pointer to data> )
 * 
 * */
uint8_t Radio_collect(struct lora_radio *self, void *data, uint8_t max)
{
    uint8_t data_size = mock_type(uint8_t);    
    (void)memcpy(data, mock_ptr_type(uint8_t *), (data_size > max) ? max : data_size);
    return data_size;
}

void Radio_interrupt(struct lora_radio *self, uint8_t n, uint64_t time)
{    
}

void Radio_setEventHandler(struct lora_radio *self, void *receiver, radioEventCB cb)
{
}

