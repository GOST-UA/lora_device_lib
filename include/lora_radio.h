#ifndef LORA_RADIO_H
#define LORA_RADIO_H

#include "lora_radio_defs.h"
#include <stdint.h>
#include <stdbool.h>

struct lora_mac;

struct lora_board {

    void (*select)(bool state);     /**< select the chip (this should also 'take' the resource if it's shared) */
    void (*reset)(bool state);      /**< true will hold the device in reset */
    void (*reset_wait)(void);       /**< block for an appropriate amount of time for reset to take effect */
    void (*write)(uint8_t data);    /**< write a byte */
    uint8_t (*read)(void);          /**< read a byte */
};

struct lora_radio {
    enum lora_radio_type {
        LORA_RADIO_SX1272
    }
    type;
    struct lora_board board;
    struct lora_mac *mac;
    union {
        struct {

        } sx1272;
    }
    state;
};

struct lora_radio_if {
    void (*init)(struct lora_radio *self, enum lora_radio_type type, struct lora_mac *mac, const struct lora_board *board);
    void (*transmit)(struct lora_radio *self, const void *data, uint8_t len);
    uint8_t (*collect)(struct lora_radio *self, void *data, uint8_t max);
    bool (*setParameters)(struct lora_radio *self, uint32_t freq, enum signal_bandwidth bw, enum spreading_factor sf);
    void (*raiseInterrupt)(struct lora_radio *self, uint8_t n);
};

void LoraRadio_init(struct lora_radio *self, enum lora_radio_type type, struct lora_mac *mac, const struct lora_board *board);
void LoraRadio_transmit(struct lora_radio *self, const void *data, uint8_t len);
void LoraRadio_receive(struct lora_radio *self);
uint8_t LoraRadio_collect(struct lora_radio *self, void *data, uint8_t max);
bool LoraRadio_setParameters(struct lora_radio *self, uint32_t freq, enum signal_bandwidth bw, enum spreading_factor sf);
void LoraRadio_raiseInterrupt(struct lora_radio *self, uint8_t n);

#endif
