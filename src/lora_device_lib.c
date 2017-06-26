#include "lora_device_lib.h"

void LoraDeviceLib_init(struct lora_device_library *self, enum lora_region_id region, enum lora_radio_type radioType, const struct lora_board *board)
{
    LORA_ASSERT(self != NULL)
    LORA_ASSERT(param != NULL)

    (void)memset(self, 0, sizeof(*self));

    ChannelList_init(&self->channels, region);
    ChannelSchedule_init(&self->schedule);
    LoraRadio_init(&self->radio, board, radioType);
    LoraMAC_init(&self->mac, &self->channels, &self->radio, self->region, &self->schedule);
}
