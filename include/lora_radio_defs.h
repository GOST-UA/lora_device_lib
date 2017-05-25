#ifndef LORA_RADIO_DEFS_H
#define LORA_RADIO_DEFS_H

/** Spreading Factor (aka symbol rate)     
 *
 * */
enum spreading_factor {
    SF_6 = 6,   /**< 64 chips/symbol */
    SF_7,       /**< 128 chips/symbol */
    SF_8,       /**< 256 chips/symbol */
    SF_9,       /**< 512 chips/symbol */
    SF_10,      /**< 1024 chips/symbol */
    SF_11,      /**< 2048 chips/symbol */
    SF_12,      /**< 4096 chips/symbol */
    SF_FSK
};

/** signal bandwidth */
enum signal_bandwidth {
    BW_125 = 125000, /**< 125 KHz */
    BW_250 = 250000, /**< 250 KHz */
    BW_500 = 500000, /**< 500 KHz */
    BW_FSK
};

enum coding_rate {
    CR_5 = 5,   /**< 4/5 */
    CR_6,       /**< 4/6 */
    CR_7,       /**< 4/7 */
    CR_8,       /**< 4/8 */
};

enum erp_setting {
    DBM_30,
    DBM_28,
    DBM_26,
    DBM_20,
    DBM_14,
    DBM_11,
    DBM_10,
    DBM_8,
    DBM_5,
    DBM_2    
};

#endif
