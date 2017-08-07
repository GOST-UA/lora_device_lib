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
#include "lora_system.h"
#include "lora_radio_sx1272.h"
#include <string.h>

/* static function prototypes *****************************************/

static void _write(struct lora_radio *self, uint8_t reg, const uint8_t *data, uint8_t len);
static void _read(struct lora_radio *self, uint8_t reg, uint8_t *data, uint8_t len);

static uint8_t readReg(struct lora_radio *self, uint8_t reg);
static uint8_t readFIFO(struct lora_radio *self, uint8_t *data, uint8_t max);
static void writeReg(struct lora_radio *self, uint8_t reg, uint8_t data);
static void writeFIFO(struct lora_radio *self, const uint8_t *data, uint8_t len);

static void setFreq(struct lora_radio *self, uint32_t freq);

static void setModemConfig(struct lora_radio *self, enum lora_signal_bandwidth bw, enum lora_spreading_factor sf, bool crc, uint16_t symbolTimeout);

static void setPower(struct lora_radio *self, int dbm);

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
    self->eventReceiver = receiver;
    self->eventHandler = cb;
    
}

uint32_t Radio_resetHardware(struct lora_radio *self)
{
    self->board->reset(self->board->receiver, true);
    System_usleep(100U);
    self->board->reset(self->board->receiver, false);
    
    return 10000U;  // should only take 5ms following reset but we want this to work on power up as well
}

bool Radio_transmit(struct lora_radio *self, const struct lora_radio_tx_setting *settings, const void *data, uint8_t len)
{
    LORA_ASSERT(self != NULL)
    LORA_ASSERT(settings != NULL)
    LORA_ASSERT((data != NULL) || (len == 0U))
    
    bool retval = false;
    
    if(len > 0U){
        
        if(((settings->bw == BW_FSK) && (settings->sf == SF_FSK)) || ((settings->bw != BW_FSK) && (settings->sf != SF_FSK))){
        
            if(settings->sf != SF_FSK){
        
                writeReg(self, RegOpMode, 0x00U);       // set sleep mode (to transition to long range mode)
                
                writeReg(self, RegOpMode, 0x81U);       // set standby mode (long range)
                
                writeReg(self, RegIrqFlags, 0xff);      // clear all interrupts
                writeReg(self, RegIrqFlagsMask, 0xf7U); // unmask TX_DONE interrupt                    
                writeReg(self, RegDioMapping1, 0x40);   // raise  DIO0 on TX_DONE
                     
                setFreq(self, settings->freq);              // set carrier frequency
    
                setPower(self, settings->erp);              // set power
                
                writeReg(self, RegSyncWord, 0x34);      // set sync word
            
                setModemConfig(self, settings->bw, settings->sf, true, settings->preamble_symbols);
                
                writeFIFO(self, data, len);                     // set pointers and write to fifo
                
                writeReg(self, RegOpMode, 0x83U);           // transmit mode
                
                retval = true;        
            }                        
        }
    }
     
    return retval;
}

bool Radio_receive(struct lora_radio *self, const struct lora_radio_rx_setting *settings)
{
    LORA_ASSERT(self != NULL)
    LORA_ASSERT(settings != NULL)
    
    bool retval = false;
    
    
    if(((settings->bw == BW_FSK) && (settings->sf == SF_FSK)) || ((settings->bw != BW_FSK) && (settings->sf != SF_FSK))){
        
        if(settings->sf != SF_FSK){
            
            writeReg(self, RegOpMode, 0x00U);       // set sleep mode (to transition to long range mode)
            
            writeReg(self, RegOpMode, 0x81U);       // set standby mode (long range)
            
            writeReg(self, RegIrqFlags, 0xff);      // clear all interrupts
            writeReg(self, RegIrqFlagsMask, 0xf7U); // unmask TX_DONE interrupt                    
            writeReg(self, RegDioMapping1, 0x40);   // raise  DIO0 on TX_DONE
                 
            setFreq(self, settings->freq);              // set carrier frequency
            
            writeReg(self, RegSyncWord, 0x34);      // set sync word
            
            writeReg(self, RegDetectOptimize, 0x03U);
            writeReg(self, RegDetectionThreshold, 0x0AU);
            
            setModemConfig(self, settings->bw, settings->sf, false, settings->preamble_symbols);
                        
            writeReg(self, RegOpMode, 0x86U);     // receive mode
            
            retval = true;        
        }
    }
    
    return retval;
}

static void setModemConfig(struct lora_radio *self, enum lora_signal_bandwidth bw, enum lora_spreading_factor sf, bool crc, uint16_t symbolTimeout)
{
    bool lowDataRateOptimize = ((bw == BW_125) && ((sf == SF_11) || (sf == SF_12))) ? true : false;
    
    uint8_t _sf;
    uint8_t _bw;
    uint8_t _cr;
    
    switch(bw){
    default:
    case BW_FSK:
    case BW_125:
        _bw = 0x00U;
        break;
    case BW_250:
        _bw = 0x40U;
        break;
    case BW_500:
        _bw = 0x80U;
        break;
    }
    
    switch(sf){
    default:
    case SF_FSK:
    case SF_7:
        _sf = 0x70U;
        break;
    case SF_8:
        _sf = 0x80U;
        break;
    case SF_9:
        _sf = 0x90U;
        break;
    case SF_10:
        _sf = 0xa0U;
        break;
    case SF_11:
        _sf = 0xb0U;
        break;
    case SF_12:
        _sf = 0xc0U;
        break;
    }
    
    _cr = 0x08U;    // CR_5
    
    
    /* burst write from RegModemConfig1 */
    uint8_t buf[] = {
        /* bandwidth (2bit)
         * coding rate (3bit)
         * implicit header (1bit) (yes)
         * crc generation and check (1bit) (no)
         * low data rate optimize (1bit) */
        _bw | _cr | 0x04U | ( crc ? 0x02U : 0x00U) | (lowDataRateOptimize ? 0x01U : 0x00U),
        /* spreading factor (4 bit)
         * tx continuous mode (1bit) (never)
         * agc auto on (1bit) (yes)
         * symbol timeout bits 9:8 */
        _sf | 0x00U | 0x04U | (((uint8_t)(symbolTimeout >> 8)) & 0x3U),
        symbolTimeout,
        0U,
        8U
    };
    
    _write(self, RegModemConfig1, buf, sizeof(buf));
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
    writeReg(self, RegOpMode, 0x00U);
}

uint8_t Radio_getRandom(struct lora_radio *self)
{
    return readReg(self, RegRssiWideband);
}

/* static functions ***************************************************/

static void setPower(struct lora_radio *self, int dbm)
{
    uint8_t outputPower;
    bool paSelect;
    uint8_t paDac;
    
#ifdef LORA_RADIO_SX1272_USE_BOOST
    
    paSelect = true;
    
    if(dbm < 5){
                
        outputPower = (uint8_t)( (dbm < 2) ? 0U : (dbm - 2) );
        paDac = 0x84U;    
    }
    else{
                
        outputPower = (uint8_t)( (dbm > 20) ? 0xfU : (dbm - 5) );
        paDac = 0x87U;    
    }
    
#else
    
    paSelect = false;
    outputPower = (dbm > 14) ? 0xfU : ( (dbm < -1) ? 0U : (dbm + 1) );
    paDac = 0x84U;

#endif
        
    writeReg(self, RegPaDac, paDac);
    writeReg(self, RegPaConfig, ( paSelect ? 0x80U : 0x00U ) | outputPower);    
}

static void setFreq(struct lora_radio *self, uint32_t freq)
{
    /* 32000000 / 2^19 */
    uint32_t f = (uint32_t) ( ((double)61.03515625) * ((double)freq) );
        
    uint8_t buf[] = {
        (f >> 16),
        (f >> 8),
        f
    };
        
    _write(self, RegFrfMsb, buf, sizeof(buf));
}

static uint8_t readFIFO(struct lora_radio *self, uint8_t *data, uint8_t max)
{
    uint8_t size = readReg(self, RegFifoRxCurrentAddr);

    size = (size > max) ? max : size;

    _read(self, RegFifo, data, size);

    return size;
}

static void writeFIFO(struct lora_radio *self, const uint8_t *data, uint8_t len)
{
    writeReg(self, RegFifoTxBaseAddr, 0x00U);   // set tx base
    writeReg(self, RegFifoAddrPtr, 0x00U);      // set address pointer

    _write(self, RegFifo, data, len);
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
