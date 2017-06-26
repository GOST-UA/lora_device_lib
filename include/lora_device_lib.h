#ifndef LORA_DEVICE_LIB_H
#define LORA_DEVICE_LIB_H

#include "lora_mac.h"
#include "lora_schedule.h"
#include "lora_channel_list.h"
#include "lora_radio.h"

struct lora_device_library {
    
    struct lora_mac mac;
    struct lora_channel_list channels;
    struct lora_schedule schedule;
    struct lora_radio radio;    
};

struct lora_mac_init {

    enum lora_region_id region;
    enum lora_radio_type radioType;
    struct lora_radio radio;
    struct lora_schedule schedule;
    struct lora_channel_list channels;
    const struct lora_board *board;
};

void LoraDeviceLib_init(struct lora_device_library *self, enum lora_region_id region, enum lora_radio_type radioType, const struct lora_board *board);

#endif
