#include "lora_radio.h"
#include "lora_debug.h"

extern const struct lora_radio_if LoraRadio_if_sx1272;

static const struct lora_radio_if *LoraRadio_if[] = {
    &LoraRadio_if_sx1272
};

/* functions **********************************************************/

void LoraRadio_init(struct lora_radio *self, enum lora_radio_type type, struct lora_mac *mac, const struct lora_board *board)
{
    LORA_ASSERT(self != NULL)
    LORA_ASSERT(mac != NULL)
    LORA_ASSERT(board != NULL)
    
    if(type < sizeof(LoraRadio_if)/sizeof(*LoraRadio_if)){

        LoraRadio_if[type]->init(self, type, mac, board);
    }
}

void LoraRadio_transmit(struct lora_radio *self, const void *data, uint8_t len)
{   
    LoraRadio_if[self->type]->transmit(self, data, len);
}

uint8_t LoraRadio_collect(struct lora_radio *self, void *data, uint8_t max)
{
    return LoraRadio_if[self->type]->collect(self, data, max);
}

bool LoraRadio_setParameters(struct lora_radio *self, uint32_t freq, enum signal_bandwidth bw, enum spreading_factor sf)
{
    return LoraRadio_if[self->type]->setParameters(self, freq, bw, sf);
}

void LoraRadio_raiseInterrupt(struct lora_radio *self, uint8_t n)
{
    LoraRadio_if[self->type]->raiseInterrupt(self, n);
}
