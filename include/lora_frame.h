#ifndef LORA_FRAME_H
#define LORA_FRAME_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* types **************************************************************/

/** recongised message types */
enum message_type {
    FRAME_TYPE_JOIN_REQ = 0,
    FRAME_TYPE_JOIN_ACCEPT,
    FRAME_TYPE_DATA_UNCONFIRMED_UP,
    FRAME_TYPE_DATA_UNCONFIRMED_DOWN,
    FRAME_TYPE_DATA_CONFIRMED_UP,
    FRAME_TYPE_DATA_CONFIRMED_DOWN,    
};

/** frame parameters */
struct lora_frame {

    enum message_type type;
    uint32_t devAddr;
    uint32_t counter;
    bool ack;
    bool adr;
    bool adrAckReq;
    bool pending;

    const uint8_t *opts;
    uint8_t optsLen;

    uint8_t port;
    const uint8_t *data;
    uint8_t dataLen;
    
};

/** the result of a decode operation */
enum lora_frame_result {
    LORA_FRAME_OK,      /**< frame is OK */
    LORA_FRAME_BAD,     /**< frame format is bad */
    LORA_FRAME_MIC      /**< frame format is OK but MIC check failed */
};

/* globals ************************************************************/

extern const size_t LORA_PHYPAYLOAD_OVERHEAD;

/* function prototypes ************************************************/

/** encode a frame
 *
 * @param[in] key   key to use for MIC and encrypt (16 byte field)
 * @param[in] f     frame parameter structure
 * @param[out] out  frame buffer
 * @param[in] max   maximum byte length of `out`
 *
 * @return bytes encoded
 *
 * @retval 0 frame could not be encoded
 *
 * */
uint8_t LoraFrame_encode(const uint8_t *key, const struct lora_frame *f, uint8_t *out, uint8_t max);

/** decode a frame
 *
 * @param[in] netKey    network key (16 byte field)
 * @param[in] appKey    application key (16 byte field)
 * @param[in] in        frame buffer (decrypt will be done in-place)
 * @param[in] len       byte length of `in`
 * @param[out] f        decoded frame structure
 *
 * @return lora_frame_result
 *
 * @retval LORA_FRAME_OK    frame decoded without any problems
 * @retval LORA_FRAME_BAD   frame format is bad
 * @retval LORA_FRAME_MIC   frame format OK but MIC check failed
 *
 * */
enum lora_frame_result LoraFrame_decode(const uint8_t *netKey, const uint8_t *appKey, uint8_t *in, uint8_t len, struct lora_frame *f);

/** calculate size of the PhyPayload
 *
 * @param[in] dataLen length of data field in bytes
 * @param[in] optsLen length of options field in bytes
 *
 * @return PhyPayload size in bytes
 *
 * */
uint16_t LoraFrame_getPhyPayloadSize(uint8_t dataLen, uint8_t optsLen);

#ifdef __cplusplus
}
#endif

#endif
