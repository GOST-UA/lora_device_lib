#ifndef LORA_CHANNEL_LIST_H
#define LORA_CHANNEL_LIST_H

#include "lora_region.h"

#ifndef LORA_NUM_CHANNELS
    #define LORA_NUM_CHANNELS    16U
#endif

#ifndef LORA_NUM_BANDS
    #define LORA_NUM_BANDS       6U
#endif

struct lora_adr_ans {
    bool channelOK;
    bool rateOK;
    bool powerOK;        
};

struct lora_channel {
    uint32_t freq;
    bool masked;
    int band;
};

struct lora_band {
    uint32_t timeReady;     ///< system time (at or after) this band will be available
    int channel;            ///< channels are cycled round robin per band so we need a reference to channel
    int numUnmasked;        ///< number of channels in this band
};

struct lora_channel_list {

    struct lora_channel channels[LORA_NUM_CHANNELS];
    struct lora_band bands[LORA_NUM_BANDS];

    enum lora_region_id region;
    const struct data_rate *rateParameters;
    enum erp_setting erp;
    
    uint8_t rate;
    uint8_t power;
    
    int nextChannel;
    int nextBand;

    int numUnmasked;    ///< total number of unmasked channels
    int numChannels;
};

void ChannelList_init(struct lora_channel_list *self, enum lora_region_id region);

bool ChannelList_add(struct lora_channel_list *self, uint32_t freq);

void ChannelList_remove(struct lora_channel_list *self, uint32_t freq);

bool ChannelList_mask(struct lora_channel_list *self, uint32_t freq);

void ChannelList_unmask(struct lora_channel_list *self, uint32_t freq);

bool ChannelList_setRateAndPower(struct lora_channel_list *self, uint8_t rate, uint8_t power);

uint32_t ChannelList_frequency(const struct lora_channel_list *self);

uint32_t ChannelList_waitTime(const struct lora_channel_list *self, uint32_t timeNow);

struct lora_adr_ans ChannelList_adrRequest(struct lora_channel_list *self, uint8_t rate, uint8_t power, uint16_t mask, uint8_t maskControl, uint8_t redundancy);

uint8_t ChannelList_maxPayload(const struct lora_channel_list *self);

void ChannelList_registerTransmission(struct lora_channel_list *self, uint32_t timeNow, uint8_t payloadLen);

enum spreading_factor ChannelList_sf(const struct lora_channel_list *self);

enum signal_bandwidth ChannelList_bw(const struct lora_channel_list *self);

enum erp_setting ChannelList_erp(const struct lora_channel_list *self);

enum coding_rate ChannelList_cr(const struct lora_channel_list *self);

size_t ChannelList_size(const struct lora_channel_list *self);

size_t ChannelList_capacity(const struct lora_channel_list *self);

#endif
