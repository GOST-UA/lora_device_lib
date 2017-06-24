#include "lora_mac.h"
#include "lora_radio.h"
#include "lora_regs_sx1272.h"
#include <string.h>

static const uint8_t spreadingFactor[] = {
    0x60U,
    0x70U,
    0x80U,
    0x90U,
    0xA0U,
    0xB0U,
    0xC0U
};

static const uint8_t signalBandwidth[] = {
    0x00U,
    0x40U,
    0x80U
};

static const uint8_t codingRate[] = {
    0x08U,
    0x10U,
    0x18U,
    0x20U,
};

enum op_mode {
    OP_SLEEP = 0,
    OP_STDBY,
    OP_FSTX,
    OP_TX,
    OP_FSRX,
    OP_RXCONTINUOUS,
    OP_RXSINGLE,
    OP_CAD
};

static const enum op_mode opModes[] = {
    OP_SLEEP,
    OP_STDBY,
    OP_FSTX,
    OP_TX,
    OP_FSRX,
    OP_RXCONTINUOUS,
    OP_RXSINGLE,
    OP_CAD
};

/* static function prototypes *****************************************/

static void setPreamble(struct lora_radio *self, uint8_t syncWord, uint16_t length);
static void write(struct lora_radio *self, uint8_t reg, const uint8_t *data, uint8_t len);
static void read(struct lora_radio *self, uint8_t reg, uint8_t *data, uint8_t len);

static uint8_t readReg(struct lora_radio *self, uint8_t reg);
static uint8_t readFIFO(struct lora_radio *self, uint8_t *data, uint8_t max);
static void writeReg(struct lora_radio *self, uint8_t reg, uint8_t data);
static void writeFIFO(struct lora_radio *self, const uint8_t *data, uint8_t len);
static void setOpMode(struct lora_radio *self, enum op_mode mode);

static void initRadio(struct lora_radio *self, enum lora_radio_type type, struct lora_mac *mac, const struct lora_board *board);
static void transmit(struct lora_radio *self, const void *data, uint8_t len);
static uint8_t collect(struct lora_radio *self, void *data, uint8_t max);
static bool setParameters(struct lora_radio *self, uint32_t freq, enum signal_bandwidth bw, enum spreading_factor sf);
static void raiseInterrupt(struct lora_radio *self, uint8_t n);

const struct lora_radio_if LoraRadio_if_sx1272 = {
    .init = initRadio,
    .transmit = transmit,
    .collect = collect,
    .setParameters = setParameters,
    .raiseInterrupt = raiseInterrupt
};

/* static functions ***************************************************/

static void initRadio(struct lora_radio *self, enum lora_radio_type type, struct lora_mac *mac, const struct lora_board *board)
{
    (void)memset(self, 0, sizeof(*self));
    (void)memcpy(&self->board, board, sizeof(*board));

    self->mac = mac;
    
    self->type = LORA_RADIO_SX1272;

    self->board.reset(true);
    self->board.reset_wait();
    self->board.reset(false);

    writeReg(self, REG_LR_OPMODE, 0U);            // sleep mode (since POR default is STDBY)
    writeReg(self, REG_LR_OPMODE, 0x80U);         // enable longRangeMode
    setPreamble(self, 0x34U, 8U);                 // lora syncword and 8 symbols of preamble
    writeReg(self, REG_LR_IRQFLAGSMASK, 0xff);    // mask all interrupts

    (void)setParameters(self, 868100000U, BW_125, SF_7);
}

static void transmit(struct lora_radio *self, const void *data, uint8_t len)
{
    if(len > 0U){

        setOpMode(self, OP_STDBY);                    // set to standby mode
        writeFIFO(self, data, len);                   // put data in fifo        
        writeReg(self, REG_LR_DIOMAPPING1, 0x40);        // raise  DIO0 on TX_DONE
        writeReg(self, REG_LR_IRQFLAGS, 0xff);        // clear all interrupts     
        writeReg(self, REG_LR_IRQFLAGSMASK, 0xf7U);   // unmask TX_DONE interrupt    
        setOpMode(self, OP_TX);                       // do transmit
    }
}

static uint8_t collect  (struct lora_radio *self, void *data, uint8_t max)
{   
    // manipulate state...
    
    return readFIFO(self, data, max);
}

/* Fxosc = 32000000
 * Fstep = Fxosc / 2^19 */
#define FREQ_STEP                                   61.03515625

static bool setParameters(struct lora_radio *self, uint32_t freq, enum signal_bandwidth bw, enum spreading_factor sf)
{
    bool retval = false;

    uint16_t rxTimeout = 5U;
    bool lowDataRateOptimize;
    bool implicitHeader;
    bool rxPayloadCRCOn = true;
    bool autoGain = true;
    enum coding_rate cr = CR_5;
    
    if((bw != BW_FSK) && (sf != SF_FSK)){

        freq = (uint32_t) ( ((double)FREQ_STEP) * ((double)freq) );
        
        uint8_t f[3U];
        f[0] = (uint8_t)freq;
        f[1] = (uint8_t)(freq >> 8);
        f[2] = (uint8_t)(freq >> 16);

        write(self, REG_LR_FRFLSB, f, sizeof(f));

        // section 4.1.1.2 sx1272 datasheet
        if(sf == SF_6){

            writeReg(self, REG_LR_DETECTOPTIMIZE, 0x05U);
            writeReg(self, REG_LR_DETECTIONTHRESHOLD, 0x0CU);
            implicitHeader = true;
        }
        else{

            writeReg(self, REG_LR_DETECTOPTIMIZE, 0x03U);
            writeReg(self, REG_LR_DETECTIONTHRESHOLD, 0x0AU);
            implicitHeader = false;
        }

        /* low data rate optimize is mandatory for this setting */
        if((bw == BW_125) && ((sf == SF_11) || (sf == SF_12))){

            lowDataRateOptimize = true;
        }
        else{

            lowDataRateOptimize = false;
        }

        writeReg(self, REG_LR_MODEMCONFIG1, signalBandwidth[bw] | codingRate[cr] | (implicitHeader ? 0x04U : 0x00U) | (rxPayloadCRCOn ? 0x02U : 0x00U ) | (lowDataRateOptimize ? 0x01U : 0x00U));            
        writeReg(self, REG_LR_MODEMCONFIG2, spreadingFactor[sf] | (autoGain ? 0x04U : 0x00U) | (((uint8_t)(rxTimeout >> 8)) & 0x3U));
        writeReg(self, REG_LR_SYMBTIMEOUTLSB, (uint8_t)rxTimeout);

        retval = true;
    }

    return retval;
}

static void raiseInterrupt(struct lora_radio *self, uint8_t n)
{    
    switch(n){
    case 0U:
        LoraMAC_eventTXComplete(self->mac);
        break;
    case 1U:
    case 2U:
    case 3U:
    default:
        break;
    }    
}

static void setPreamble(struct lora_radio *self, uint8_t syncWord, uint16_t length)
{
    writeReg(self, REG_LR_SYNCWORD, syncWord);

    uint8_t s[2U];
    s[0] = (uint8_t)length;
    s[1] = (uint8_t)(length >> 8);
    
    write(self, REG_LR_PREAMBLELSB, s, sizeof(s));            
}

static void write(struct lora_radio *self, uint8_t reg, const uint8_t *data, uint8_t len)
{
    uint8_t i;

    if(len > 0){

        self->board.select(true);

        self->board.write(reg | 0x80U);

        for(i=0; i < len; i++){

            self->board.write(data[i]);
        }

        self->board.select(false);
    }
}

static void read(struct lora_radio *self, uint8_t reg, uint8_t *data, uint8_t len)
{
    uint8_t i;

    if(len > 0){

        self->board.select(true);

        self->board.write(reg & 0x7fU);

        for(i=0; i < len; i++){

            data[i] = self->board.read();
        }

        self->board.select(false);
    }
}

static uint8_t readReg(struct lora_radio *self, uint8_t reg)
{
    uint8_t data;
    read(self, reg, &data, sizeof(data));
    return data;
}

static uint8_t readFIFO(struct lora_radio *self, uint8_t *data, uint8_t max)
{
    uint8_t size = readReg(self, REG_LR_FIFOADDRPTR);

    size = (size > max) ? max : size;

    read(self, REG_LR_FIFORXCURRENTADDR, data, size);

    return size;
}

static void writeReg(struct lora_radio *self, uint8_t reg, uint8_t data)
{
    read(self, reg, &data, sizeof(data));
}

static void writeFIFO(struct lora_radio *self, const uint8_t *data, uint8_t len)
{
    uint8_t i;

    /* reset fifo pointer to txbase (so chip can work out size of data) */
    writeReg(self, REG_LR_FIFOADDRPTR, 0x00U);

    for(i=0U; i < len; i++){

        writeReg(self, REG_LR_FIFOADDRPTR, data[i]);
    }
}

static void setOpMode(struct lora_radio *self, enum op_mode mode)
{
    writeReg(self, REG_LR_OPMODE, mode);
}


