#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "cmocka.h"
#include "lora_mac_commands.h"

bool MAC_putLinkCheckReq(struct lora_stream *s)
{
    return mock_type(bool);
}

bool MAC_putLinkCheckAns(struct lora_stream *s, const struct lora_link_check_ans *value)
{
    return mock_type(bool);
}

bool MAC_putLinkADRReq(struct lora_stream *s, const struct lora_link_adr_req *value)
{
    return mock_type(bool);
}

bool MAC_putLinkADRAns(struct lora_stream *s, const struct lora_link_adr_ans *value)
{
    return mock_type(bool);
}

bool MAC_putDutyCycleReq(struct lora_stream *s, const struct lora_duty_cycle_req *value)
{
    return mock_type(bool);
}

bool MAC_putDutyCycleAns(struct lora_stream *s)
{
    return mock_type(bool);
}

bool MAC_putRXParamSetupReq(struct lora_stream *s, const struct lora_rx_param_setup_req *value)
{
    return mock_type(bool);
}

bool MAC_putDevStatusReq(struct lora_stream *s)
{
    return mock_type(bool);
}

bool MAC_putDevStatusAns(struct lora_stream *s, const struct lora_dev_status_ans *value)
{
    return mock_type(bool);
}

bool MAC_putNewChannelReq(struct lora_stream *s, const struct lora_new_channel_req *value)
{
    return mock_type(bool);
}

bool MAC_putRXParamSetupAns(struct lora_stream *s, const struct lora_rx_param_setup_ans *value)
{
    return mock_type(bool);
}

bool MAC_putNewChannelAns(struct lora_stream *s, const struct lora_new_channel_ans *value)
{
    return mock_type(bool);
}

bool MAC_putDLChannelReq(struct lora_stream *s, const struct lora_dl_channel_req *value)
{
    return mock_type(bool);
}

bool MAC_putDLChannelAns(struct lora_stream *s, const struct lora_dl_channel_ans *value)
{
    return mock_type(bool);
}

bool MAC_putRXTimingSetupReq(struct lora_stream *s, const struct lora_rx_timing_setup_req *value)
{
    return mock_type(bool);
}

bool MAC_putRXTimingSetupAns(struct lora_stream *s)
{
    return mock_type(bool);
}

bool MAC_putTXParamSetupReq(struct lora_stream *s, const struct lora_tx_param_setup_req *value)
{
    return mock_type(bool);
}

bool MAC_putTXParamSetupAns(struct lora_stream *s)
{
    return mock_type(bool);
}

bool MAC_eachDownstreamCommand(void *receiver, const uint8_t *data, uint8_t len, void (*handler)(void *, const struct lora_downstream_cmd *))
{
    return mock_type(bool);
}

bool MAC_eachUpstreamCommand(void *receiver, const uint8_t *data, uint8_t len, void (*handler)(void *, const struct lora_upstream_cmd *))
{
    return mock_type(bool);
}
