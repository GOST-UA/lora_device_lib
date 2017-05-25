#ifndef LORA_MAC_H
#define LORA_MAC_H

#include "lora_channel_list.h"
#include "lora_radio.h"

struct lora_mac {

    uint8_t devEUI[8U];
    uint8_t appEUI[8U];
    
    uint8_t appKey[16U];

    uint8_t nwkSKey[16U];
    uint8_t appSKey[16U];

    uint16_t rwindow;
    uint32_t upCounter;

    uint8_t buffer[UINT8_MAX];
    uint8_t bufferLen;

    uint32_t devAddr;

    struct lora_channel_list channels;

    struct lora_radio radio;

};

struct lora_mac_init {

    enum lora_region_id region;
    enum lora_radio_type radioType;
    const struct lora_board *board;
};

enum lora_tx_status {
    LORA_TX_COMPLETE,           ///< non-confirmed upstream tx operation is complete
    LORA_TX_CONFIRMED,          ///< tx operation complete and confirmation received
    LORA_TX_CONFIRM_TIMEOUT     ///< tx operation complete but confirmation never received
};

typedef void (*txCompleteCB)(struct lora_mac *mac, void *user, enum lora_tx_status status);

void LoraMAC_init(struct lora_mac *self, struct lora_mac_init *param);
void LoraMAC_setSession(struct lora_mac *self, uint32_t devAddr, const uint8_t *nwkSKey, const uint8_t *appSKey);
bool LoraMAC_addChannel(struct lora_mac *self, uint32_t freq);
void LoraMAC_removeChannel(struct lora_mac *self, uint32_t freq);
bool LoraMAC_maskChannel(struct lora_mac *self, uint32_t freq);
void LoraMAC_unmaskChannel(struct lora_mac *self, uint32_t freq);
bool LoraMAC_setRateAndPower(struct lora_mac *self, uint8_t rate, uint8_t power);
bool LoraMAC_unconfirmedUp(struct lora_mac *self, uint8_t port, const void *data, uint8_t len, txCompleteCB cb);
void LoraMAC_eventTXComplete(struct lora_mac *self);


#endif
