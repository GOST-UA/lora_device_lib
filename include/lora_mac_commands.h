#if 0

#ifndef LORA_MAX_COMMANDS_H
#define LORA_MAX_COMMANDS_H

#include <stdint.h>
#include <stdbool.h>

struct stream {
  
    const uint8_t *buf;
    uint8_t *size;    
    uint8_t pos;    
};

enum mac_cmd_type {
    
    LINK_CHECK,
    LINK_ADR,
    DUTY_CYCLE,
    RX_PARAM_SETUP,
    DEV_STATUS,
    NEW_CHANNEL,
    RX_TIMING_SETUP,
    TX_PARAM_SETUP,
    DL_CHANNEL,
    
    PING_SLOT_INFO,
    PING_SLOT_CHANNEL,
    PING_SLOT_FREQ,
    BEACON_TIMING,
    BEACON_FREQ,
};

struct link_check_ans {
                
    uint8_t Margin;
    uint8_t gwCount;
};

struct link_adr_req {
    
    uint8_t dataRate;
    uint8_t txPower;
    uint16_t channelMask;
    uint8_t channelMaskControl;
    uint8_t NbTrans;        
};

struct link_adr_ans {
    
    bool powerOK;
    bool dataRateOK;
    bool channelMaskOK;
};

struct duty_cycle_req {
    
    uint8_t maxDutyCycle;
};

struct rx_param_setup_req {
    
    uint8_t rx1DROffset    
    uint8_t rx2DataRate;
    uint32_t freq;
};

struct rx_param_setup_ans {
    
    bool rx1DROffsetOK;
    bool rx2DataRateOK;
    bool freqOK;
};

struct dev_status_ans {
    
    uint8_t battery;
    uint8_t margin;
};

struct new_channel_req {
    
    uint8_t chIndex;
    uint32_t freq;
    uint8_t maxDR;
    uint8_t minDR;
};
    
struct new_channel_ans {
    
    bool dataRateRangeOK;
    bool channelFrequencyOK;
};

struct dl_channel_req {
    
    uint8_t chIndex;
    uint32_t freq;    
};

struct dl_channel_ans {
    
    bool uplinkFreqOK;
    bool channelFrequencyOK;
};

struct rx_timing_setup_req {
    
    uint8_t delay;
};

struct tx_param_setup_req {
    
    bool downlinkDwell;
    bool uplinkDwell;
    uint8_t eirp;
};

struct device_cmd_req {
  
    enum mac_cmd_type type;
  
    union {
      
        struct link_check_ans linkCheckAns;
        struct link_adr_req linkADRReq;
        struct duty_cycle_req dutyCycleReq;
        struct rx_param_setup_req rxParamSetupReq;
        /* dev_status_req */
        struct new_channel_req newChannelReq;
        struct dl_channel_req dlChannelReq;
        struct rx_timing_setup_req rxTimingSetupReq;
        struct tx_param_setup_req txParamSetupReq;
        
    } fields;    
};

struct network_cmd_req {
  
    enum mac_cmd_type type;
  
    union {
      
        struct link_adr_req linkADRReq;
        /* duty_cycle_ans */        
        struct rx_param_setup_ans rxParamSetupAns;
        struct dev_status_ans rxParamSetupAns;
        struct new_channel_ans newChannelAns;
        struct dl_channel_ans dlChannelAns;
        /* rx_timing_setup_ans */
        /* tx_param_setup_ans */
        
    } fields;    
};

bool putLinkCheckReq(struct stream *s);
bool putLinkCheckAns(struct stream *s, const struct link_adr_req *value);
bool putLinkADRReq(struct stream *s, const struct link_adr_req *value);
bool putLinkADRAns(struct stream *s, const struct link_adr_ans *value);
bool putDutyCycleReq(struct stream *s, const struct duty_cycle_req *value);
bool putDutyCycleAns(struct stream *s);
bool putRXParamSetupReq(struct stream *s, const struct rx_param_setup_req *value);
bool putDeviceStatusReq(struct stream *s);
bool putDeviceStatusAns(struct stream *s, const struct dev_status_ans *value);
bool putNewChannelReq(struct stream *s, const struct new_channel_req *value);
bool putRXParamSetupAns(struct stream *s, const struct rx_param_setup_ans *value);
bool putNewChannelAns(struct stream *s, const struct new_channel_ans *value);
bool putDLChannelReq(struct stream *s, const struct dl_channel_req *value);
bool putDLChannelAns(struct stream *s, const struct dl_channel_ans *value);
bool putRXTimingSetupReq(struct stream *s, const struct rx_timing_setup_req *value);
bool putRXTimingSetupAns(struct stream *s);
bool putTXParamSetupReq(struct stream *s, const struct tx_param_setup_req *value);
bool putTXParamSetupAns(struct stream *s);

#endif

#endif
