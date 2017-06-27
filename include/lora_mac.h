#ifndef LORA_MAC_H
#define LORA_MAC_H

#include "lora_region.h"

#include <stdint.h>
#include <stdbool.h>

struct lora_radio;
struct lora_event;
struct lora_channeL_list;
struct lora_mac;

/* shift to region */
#define INTERVAL_RX1    1000
#define INTERVAL_RX2    1000

enum lora_tx_status {
    LORA_TX_COMPLETE,           ///< non-confirmed upstream tx operation is complete
    LORA_TX_CONFIRMED,          ///< tx operation complete and confirmation received
    LORA_TX_CONFIRM_TIMEOUT     ///< tx operation complete but confirmation never received
};

/** Transmit Complete Notification
 * 
 * @param[in] self user receiver
 * @param[in] mac
 * @param[in] status
 * 
 * */
typedef void (*txCompleteCB)(void *receiver, struct lora_mac *mac, enum lora_tx_status status);

/** Receive Complete Notification
 * 
 * - receive must make a copy data or else it will be lost
 * 
 * @param[in] self user receiver
 * @param[in] mac 
 * @param[in] port LoRaWAN port
 * @param[in] data
 * @param[in] len byte length of data
 * 
 * */
typedef void (*rxCompleteCB)(void *receiver, struct lora_mac *mac, uint8_t port, const void *data, uint8_t len);

enum states {

    IDLE,
    WAIT_TX,
    TX,         // radio is TX
    WAIT_RX1,   // waiting for first RX window
    RX1,        // first RX window
    WAIT_RX2,   // waiting for second RX window
    RX2,        // second RX window
    BEACON_RX,  // beacon window        
    ERROR
};

struct lora_mac {

    enum states state;
    
    uint8_t txCount;
    bool confirmed;

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
    
    uint8_t nbTrans;    /**< number of transmissions for each uplink message */
    
    uint32_t rx1_interval;
    uint32_t rx2_interval;
    
    uint8_t macAns[16U];
    uint8_t macAnsLen;
    
    #define RX_WDT_INTERVAL 60000
    
    struct lora_channel_list *channels;
    struct lora_radio *radio;
    struct lora_event *events;
    
    void *rxReady;
    void *rxTimeout;
    void *txComplete;
    void *resetRadio;
    
    struct {
        
        uint32_t time;
        uint8_t gateway_count;
        uint8_t margin;                
    
    }lastLinkCheck;
    
    
    
    
    struct {
        
        bool LinkADRAns_pending;
        uint8_t LinkADRAns[1];
        
        bool DutyCycleAns_pending;
        
        bool RXParamSetupAns_pending;
        uint8_t RXParamSetupAns[1];
        
        bool DevStatusAns_pending;
        uint8_t DevStatusAns[2];
        
        bool NewChannelAns_pending;
        uint8_t NewChannelAns[1];
        
        bool RXTimingSetupAns_pending;
        
        bool TXParamSetupAns_pending;
        
        bool DlChannelAns_pending;
        uint8_t DlChannelAns[1]; 
        
    } cmd;
    
    rxCompleteCB rxCompleteHandler;
    void *rxCompleteReceiver;
    
    txCompleteCB txCompleteHandler;
    void *txCompleteReceiver;
};

void LoraMAC_init(struct lora_mac *self, struct lora_channel_list *channels, struct lora_radio *radio, struct lora_event *events);

void LoraMAC_setSession(struct lora_mac *self, uint32_t devAddr, const void *nwkSKey, const void *appSKey);
bool LoraMAC_addChannel(struct lora_mac *self, uint8_t chIndex, uint32_t freq);
void LoraMAC_removeChannel(struct lora_mac *self, uint8_t chIndex);
bool LoraMAC_maskChannel(struct lora_mac *self, uint8_t chIndex);
void LoraMAC_unmaskChannel(struct lora_mac *self, uint8_t chIndex);
bool LoraMAC_setRateAndPower(struct lora_mac *self, uint8_t rate, uint8_t power);

/** propagate tx complete event */
void LoraMAC_eventTXComplete(struct lora_mac *self, uint32_t time);

/** propagate rx ready event */
void LoraMAC_eventRXReady(struct lora_mac *self, uint32_t time);

/** propagate rx timeout event */
void LoraMAC_eventRXTimeout(struct lora_mac *self, uint32_t time);

/** Send a message upstream
 * 
 * Call may be rejected for the following reasons:
 * 
 * - MAC is already sending (LoraMAC_txPending() returns true)
 * - Message is too large
 * 
 * If the call is accepted it may be some time before it completes. The callback
 * confirmation is provided for the purpose of synchronisation. Sources of
 * delay include:
 * 
 * - Duty cycle limits on available channels
 * - Retransmission depending on MAC setting
 * 
 * @param[in] self
 * @param[in] confirmed true if this send shoudl be confirmed
 * @param[in] port 
 * @param[in] data pointer to message to send (will be cached by MAC)
 * @param[in] len byte length of data
 * @param[in] user tx complate notification receiver
 * @param[in] cb tx complete notification method
 * 
 * @retval true unconfirmed up is possible and now pending
 * 
 * */
bool LoraMAC_send(struct lora_mac *self, bool confirmed, uint8_t port, const void *data, uint8_t len, void *receiver, txCompleteCB cb);
 

/**
 * A MAC will handle one upstream request at a time
 * 
 * @param[in] self 
 * @return true if mac is handling a upstream request 
 * 
 * */
bool LoraMAC_sendIsPending(struct lora_mac *self);

/** 
 * MAC will call this handler when a message is received downstream
 * 
 * @param[in] self
 * @param[in] user callback receiver
 * @param[in] cb handler
 * 
 * */
void LoraMAC_setReceiveHandler(struct lora_mac *self, void *receiver, rxCompleteCB cb);

bool LoraMAC_setNbTrans(struct lora_mac *self, uint8_t nbTrans);

#endif
