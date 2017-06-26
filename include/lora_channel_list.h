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
    int band;
    bool masked;
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
};

void ChannelList_init(struct lora_channel_list *self, enum lora_region_id region);

/** Add a (transmission) channel
 * 
 * @param[in] self
 * @param[in] chIndex   channel index (0..LORA_NUM_CHANNELS-1)
 * @param[in] freq      frequency in Hz (0 has same effect as remove)
 * 
 * @return true if channel could be added
 * 
 * */
bool ChannelList_add(struct lora_channel_list *self, uint8_t chIndex, uint32_t freq);

/** Remove a (transmission) channel
 * 
 * @param[in] self
 * @param[in] chIndex channel index (0..LORA_NUM_CHANNELS)
 * 
 * */
void ChannelList_remove(struct lora_channel_list *self, uint8_t chIndex);

bool ChannelList_mask(struct lora_channel_list *self, uint8_t chIndex);

void ChannelList_unmask(struct lora_channel_list *self, uint8_t chIndex);

bool ChannelList_setRateAndPower(struct lora_channel_list *self, uint8_t rate, uint8_t power);

uint32_t ChannelList_waitTime(const struct lora_channel_list *self, uint32_t timeNow);

struct lora_adr_ans ChannelList_adrRequest(struct lora_channel_list *self, uint8_t rate, uint8_t power, uint16_t mask, uint8_t maskControl, uint8_t redundancy);

void ChannelList_registerTransmission(struct lora_channel_list *self, uint32_t timeNow, uint8_t payloadLen);

size_t ChannelList_capacity(const struct lora_channel_list *self);

struct lora_channel_setting {
  
  uint32_t freq;
  enum spreading_factor sf;
  enum signal_bandwidth bw;
  enum erp_setting erp;
  enum coding_rate cr;      
  uint8_t maxPayload;
  
};

bool ChannelList_upstreamSetting(const struct lora_channel_list *self, struct lora_channel_setting *setting);
bool ChannelList_rx1Setting(const struct lora_channel_list *self, struct lora_channel_setting *setting);
bool ChannelList_rx2Setting(const struct lora_channel_list *self, struct lora_channel_setting *setting);
bool ChannelList_beaconSetting(const struct lora_channel_list *self, struct lora_channel_setting *setting);

#endif
