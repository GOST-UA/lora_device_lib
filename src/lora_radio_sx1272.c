/* Copyright (c) 2017 Cameron Harper
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * */

#include "lora_debug.h"
#include "lora_radio.h"
#include "lora_board.h"
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

/* static function prototypes *****************************************/

static void _write(struct lora_radio *self, uint8_t reg, const uint8_t *data, uint8_t len);
static void _read(struct lora_radio *self, uint8_t reg, uint8_t *data, uint8_t len);

static uint8_t readReg(struct lora_radio *self, uint8_t reg);
static uint8_t readFIFO(struct lora_radio *self, uint8_t *data, uint8_t max);
static void writeReg(struct lora_radio *self, uint8_t reg, uint8_t data);
static void writeFIFO(struct lora_radio *self, const uint8_t *data, uint8_t len);

static bool setParameters(struct lora_radio *self, const struct lora_radio_setting *settings);

/* functions **********************************************************/

void Radio_init(struct lora_radio *self, const struct lora_board *board)
{
    (void)memset(self, 0, sizeof(*self));
    self->board = board;
}

void Radio_setEventHandler(struct lora_radio *self, void *receiver, radioEventCB cb)
{
    LORA_ASSERT(self != NULL)
    
    // todo: this is a critical section
    self->eventHandler = cb;
    self->eventReceiver = receiver;
}

uint32_t Radio_resetHardware(struct lora_radio *self)
{
    self->board->reset(self->board->receiver, true);
    //wait > 100us
    self->board->reset(self->board->receiver, false);
    
    return 10000U;  // should only take 5ms following reset but we want this to work on power up as well
}

void Radio_resetState(struct lora_radio *self)
{
    writeReg(self, REG_LR_OPMODE, 0x00U);           // sleep mode (since POR default is STDBY)
    writeReg(self, REG_LR_OPMODE, 0x80U);           // enable longRangeMode
    writeReg(self, REG_LR_SYNCWORD, 0x34);          // set lora sync word       
    writeReg(self, REG_LR_IRQFLAGSMASK, 0xff);      // mask all interrupts
    writeReg(self, REG_LR_FIFOTXBASEADDR, 0x00U);   // set tx base address to 0 (RX is already at 0)
}

bool Radio_transmit(struct lora_radio *self, const struct lora_radio_setting *settings, const void *data, uint8_t len)
{
    LORA_ASSERT(self != NULL)
    LORA_ASSERT(settings != NULL)
    LORA_ASSERT((data != NULL) || (len == 0U))
    
    bool retval = false;
    
    if(len > 0U){
        
        if(setParameters(self, settings)){

            writeReg(self, REG_LR_OPMODE, 0x81U);           // set to standby mode
            writeFIFO(self, data, len);                     // put data in fifo        
            writeReg(self, REG_LR_DIOMAPPING1, 0x40);       // raise  DIO0 on TX_DONE
            writeReg(self, REG_LR_IRQFLAGS, 0xff);          // clear all interrupts     
            writeReg(self, REG_LR_IRQFLAGSMASK, 0xf7U);     // unmask TX_DONE interrupt    
            writeReg(self, REG_LR_OPMODE, 0x83U);           // set to transmit
            
            retval = true;
        }
    }
    
    return retval;
}

bool Radio_receive(struct lora_radio *self, bool continuous, const struct lora_radio_setting *settings)
{
    LORA_ASSERT(self != NULL)
    LORA_ASSERT(settings != NULL)
    
    bool retval = false;
    
    if(setParameters(self, settings)){
        
        retval = false;
    }
    
    return retval;
}

uint8_t Radio_collect(struct lora_radio *self, void *data, uint8_t max)
{   
    LORA_ASSERT(self != NULL)
    LORA_ASSERT((data != NULL) || (max == 0U))
    
    return readFIFO(self, data, max);
}

void Radio_raiseInterrupt(struct lora_radio *self, uint8_t n, uint64_t time)
{    
    LORA_ASSERT(self != NULL)
    
    switch(n){
    case 0U:
        if(self->eventHandler != NULL){
            
            self->eventHandler(self->eventReceiver, LORA_RADIO_TX_COMPLETE, time);
        }
        break;
    case 1U:
    case 2U:
    case 3U:
    default:
        break;
    }    
}

void Radio_sleep(struct lora_radio *self)
{
    writeReg(self, REG_LR_OPMODE, 0x80U);
}

/* static functions ***************************************************/

/* Fxosc = 32000000
 * Fstep = Fxosc / 2^19 */
#define FREQ_STEP                                   61.03515625

static bool setParameters(struct lora_radio *self, const struct lora_radio_setting *settings)
{
    bool retval = false;

    uint16_t rxTimeout = 5U;
    bool lowDataRateOptimize;
    bool implicitHeader;
    bool rxPayloadCRCOn = true;
    bool autoGain = true;
    enum lora_coding_rate cr = CR_5;
    
    if((settings->bw != BW_FSK) && (settings->sf != SF_FSK)){

        uint32_t freq = (uint32_t) ( ((double)FREQ_STEP) * ((double)settings->freq) );
        
        uint8_t f[3U];
        f[0] = (uint8_t)freq;
        f[1] = (uint8_t)(freq >> 8);
        f[2] = (uint8_t)(freq >> 16);

        _write(self, REG_LR_FRFLSB, f, sizeof(f));

        // section 4.1.1.2 sx1272 datasheet
        if(settings->sf == SF_6){

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
        if((settings->bw == BW_125) && ((settings->sf == SF_11) || (settings->sf == SF_12))){

            lowDataRateOptimize = true;
        }
        else{

            lowDataRateOptimize = false;
        }

        writeReg(self, REG_LR_MODEMCONFIG1, signalBandwidth[settings->bw] | codingRate[cr] | (implicitHeader ? 0x04U : 0x00U) | (rxPayloadCRCOn ? 0x02U : 0x00U ) | (lowDataRateOptimize ? 0x01U : 0x00U));            
        writeReg(self, REG_LR_MODEMCONFIG2, spreadingFactor[settings->sf] | (autoGain ? 0x04U : 0x00U) | (((uint8_t)(rxTimeout >> 8)) & 0x3U));
        writeReg(self, REG_LR_SYMBTIMEOUTLSB, (uint8_t)rxTimeout);

        retval = true;
    }

    return retval;
}


static uint8_t readFIFO(struct lora_radio *self, uint8_t *data, uint8_t max)
{
    uint8_t size = readReg(self, REG_LR_FIFORXCURRENTADDR);

    size = (size > max) ? max : size;

    _read(self, REG_LR_FIFO, data, size);

    return size;
}

static void writeFIFO(struct lora_radio *self, const uint8_t *data, uint8_t len)
{
    writeReg(self, REG_LR_FIFOADDRPTR, 0x00U);

    _write(self, REG_LR_FIFO, data, len);
}

static uint8_t readReg(struct lora_radio *self, uint8_t reg)
{
    uint8_t data;
    _read(self, reg, &data, sizeof(data));
    return data;
}

static void writeReg(struct lora_radio *self, uint8_t reg, uint8_t data)
{
    _write(self, reg, &data, sizeof(data));
}

static void _write(struct lora_radio *self, uint8_t reg, const uint8_t *data, uint8_t len)
{
    uint8_t i;

    if(len > 0U){

        self->board->select(self->board->receiver, true);

        self->board->write(self->board->receiver, reg | 0x80U);

        for(i=0; i < len; i++){

            self->board->write(self->board->receiver, data[i]);
        }

        self->board->select(self->board->receiver, false);
    }
}

static void _read(struct lora_radio *self, uint8_t reg, uint8_t *data, uint8_t len)
{
    uint8_t i;

    if(len > 0U){

        self->board->select(self->board->receiver, true);

        self->board->write(self->board->receiver, reg & 0x7fU);

        for(i=0U; i < len; i++){

            data[i] = self->board->read(self->board->receiver);
        }

        self->board->select(self->board->receiver, false);
    }
}
