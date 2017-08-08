#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"

#include "lora_radio.h"
#include "lora_radio_sx1272.h"
#include "mock_board.h"

#include <string.h>


/**********************************************************************/

#if 0
static void handle_event(void *receiver, enum lora_radio_event event, uint64_t time)
{
}
#endif

void System_usleep(uint32_t interval)
{
}

/**********************************************************************/

static int setup_preInit(void **user)
{
    static struct user_data u;
    (void)memset(&u, 0, sizeof(u));
    
    u.board.receiver = &u.chip;
    u.board.select = board_select;
    u.board.reset = board_reset;
    u.board.write = board_write;
    u.board.read = board_read;
    
    *user = &u;
    
    return 0;
}

static int setup_postInit(void **user)
{
    struct user_data *u;
    
    (void)setup_preInit(user);
    
    u = (struct user_data *)(*user);
    
    Radio_init(&u->radio, &u->board);
    
    return 0;
}

static void test_Radio_init(void **user)
{
    struct user_data *u = (struct user_data *)(*user);
    
    Radio_init(&u->radio, &u->board);
}

static void test_Radio_resetHardware(void **user)
{
    struct user_data *u = (struct user_data *)(*user);
    
    /* Radio_init will perform a chip reset */
    
    expect_value(board_reset, state, true);
    expect_value(board_reset, state, false);    
    
    Radio_resetHardware(&u->radio);
}

static void test_Radio_transmit_noPayload(void **user)
{
    struct user_data *u = (struct user_data *)(*user);        
    
    struct lora_radio_tx_setting setting = {
        .freq = 0,
        .bw = BW_125,
        .sf = SF_7,
        .cr = CR_5,
        .power = 14
    };
    
    Radio_transmit(&u->radio, &setting, NULL, 0U);
}

static void test_Radio_transmit(void **user)
{
    struct user_data *u = (struct user_data *)(*user);        
    size_t i;
    const uint8_t payload[] = {0x00U, 0x01U, 0x03U};
    
    struct lora_radio_tx_setting setting = {
        .freq = 0,
        .bw = BW_125,
        .sf = SF_7,
        .cr = CR_5,
        .power = 14,
        .preamble = 8
    };
    
    /* set RegOpMode to sleep mode */
    expect_value(board_select, state, true);    
    expect_value(board_write, data, 0x80U | RegOpMode);
    expect_value(board_write, data, 0x00U);
    expect_value(board_select, state, false);  
    
    /* set RegOpMode to standby mode */
    expect_value(board_select, state, true);    
    expect_value(board_write, data, 0x80U | RegOpMode);
    expect_value(board_write, data, 0x81U);
    expect_value(board_select, state, false);  
    
    /* set RegIrqFlags to clear all interrupt flags */
    expect_value(board_select, state, true);    
    expect_value(board_write, data, 0x80U | RegIrqFlags);
    expect_value(board_write, data, 0xffU);
    expect_value(board_select, state, false);  
    
    /* set RegIrqFlagsMask to unmask TX_DONE interrupt */
    expect_value(board_select, state, true);    
    expect_value(board_write, data, 0x80U | RegIrqFlagsMask);
    expect_value(board_write, data, 0xf7U);
    expect_value(board_select, state, false);  
    
    /* set REG_LR_DIOMAPPING1 to raise DIO0 on TX DONE */
    expect_value(board_select, state, true);    
    expect_value(board_write, data, 0x80U | RegDioMapping1);
    expect_value(board_write, data, 0x01U);
    expect_value(board_select, state, false);  
    
    /* freq */
    expect_value(board_select, state, true);    
    expect_value(board_write, data, 0x80U | RegFrfMsb);
    expect_value(board_write, data, 0x00U);
    expect_value(board_write, data, 0x00U);
    expect_value(board_write, data, 0x00U);
    expect_value(board_select, state, false);  
    
    /* power dac */
    expect_value(board_select, state, true);    
    expect_value(board_write, data, 0x80U | RegPaDac);
    expect_value(board_write, data, 0x84U);
    expect_value(board_select, state, false);  
    
    /* power config */
    expect_value(board_select, state, true);    
    expect_value(board_write, data, 0x80U | RegPaConfig);
    expect_value(board_write, data, 0x0fU);
    expect_value(board_select, state, false);  
    
    /* SYNCWORD */
    expect_value(board_select, state, true);    
    expect_value(board_write, data, 0x80U | RegSyncWord);
    expect_value(board_write, data, 0x34U);
    expect_value(board_select, state, false);  
    
    /* RegModemConfig1, RegModemConfig2,  
     * 
     * RegModemConfig1 = Bw (7..6) | CodingRate (5..3) | ImplicitHeader (2) | Crc (1) | LowDateRateOptimise (0)
     * 
     * RegModemConfig2 = SpreadingFactor (7..4) | TxContinuousMode (3) | AgcAutoOn (2) | SymbTimeout (1..0) (MSB)
     * 
     * RegSymbTimeoutLSB (7..0)
     * 
     * RegPreambleMsb
     * RegPreambleLsb
     * 
     * */
    expect_value(board_select, state, true);    
    expect_value(board_write, data, 0x80U | RegModemConfig1);
    expect_value(board_write, data, 0x00U | 0x08U | 0x04U | 0x02U);
    expect_value(board_write, data, 0x70U | 0x04U | 0x00U);
    expect_value(board_write, data, 0x00U);
    expect_value(board_write, data, 0x00U);
    expect_value(board_write, data, 0x08U);
    expect_value(board_select, state, false);  
    
    /* LNA register */
    expect_value(board_select, state, true);    
    expect_value(board_write, data, 0x80U | RegLna);
    expect_value(board_write, data, 0x60U);
    expect_value(board_select, state, false);  
    
    expect_value(board_select, state, true);    
    expect_value(board_write, data, 0x80U | RegFifoTxBaseAddr);
    expect_value(board_write, data, 0x00U);
    expect_value(board_select, state, false);  
    
    /* set REG_LR_FIFOADDRPTR to the base address (0) */
    expect_value(board_select, state, true);    
    expect_value(board_write, data, 0x80U | RegFifoAddrPtr);
    expect_value(board_write, data, 0x00U);
    expect_value(board_select, state, false);  
    
    /* burst write the FIFO */
    expect_value(board_select, state, true);    
    expect_value(board_write, data, 0x80U | RegFifo);    
    for(i=0U; i < sizeof(payload); i++){
        expect_value(board_write, data, payload[i]);
    }        
    expect_value(board_select, state, false);  
    
    /* set RegOpMode to single transmit */
    expect_value(board_select, state, true);    
    expect_value(board_write, data, 0x80U | RegOpMode);
    expect_value(board_write, data, 0x83U);
    expect_value(board_select, state, false);  
        
    Radio_transmit(&u->radio, &setting, payload, sizeof(payload));
}

static void test_Radio_receive(void **user)
{
    struct user_data *u = (struct user_data *)(*user);        
    
    struct lora_radio_rx_setting setting = {
        .freq = 0,
        .bw = BW_125,
        .sf = SF_7,
        .cr = CR_5,
        .preamble = 8, 
        .timeout = 0, 
    };
    
    /* set RegOpMode to sleep mode */
    expect_value(board_select, state, true);    
    expect_value(board_write, data, 0x80U | RegOpMode);
    expect_value(board_write, data, 0x00U);
    expect_value(board_select, state, false);  
    
    /* set RegOpMode to standby mode */
    expect_value(board_select, state, true);    
    expect_value(board_write, data, 0x80U | RegOpMode);
    expect_value(board_write, data, 0x81U);
    expect_value(board_select, state, false);  
    
    /* set RegIrqFlags to clear all interrupt flags */
    expect_value(board_select, state, true);    
    expect_value(board_write, data, 0x80U | RegIrqFlags);
    expect_value(board_write, data, 0xffU);
    expect_value(board_select, state, false);  
    
    /* set RegIrqFlagsMASK to unmask TX_DONE interrupt */
    expect_value(board_select, state, true);    
    expect_value(board_write, data, 0x80U | RegIrqFlagsMask);
    expect_value(board_write, data, 0xf7U);
    expect_value(board_select, state, false);  
    
    /* set REG_LR_DIOMAPPING1 */
    expect_value(board_select, state, true);    
    expect_value(board_write, data, 0x80U | RegDioMapping1);
    expect_value(board_write, data, 0x00);
    expect_value(board_select, state, false);  
    
    /* freq */
    expect_value(board_select, state, true);    
    expect_value(board_write, data, 0x80U | RegFrfMsb);
    expect_value(board_write, data, 0x00U);
    expect_value(board_write, data, 0x00U);
    expect_value(board_write, data, 0x00U);
    expect_value(board_select, state, false);  
    
    /* SYNCWORD */
    expect_value(board_select, state, true);    
    expect_value(board_write, data, 0x80U | RegSyncWord);
    expect_value(board_write, data, 0x34U);
    expect_value(board_select, state, false);  
    
    expect_value(board_select, state, true);    
    expect_value(board_write, data, 0x80U | RegDetectOptimize);
    expect_value(board_write, data, 0x03U);
    expect_value(board_select, state, false);  
    
    expect_value(board_select, state, true);    
    expect_value(board_write, data, 0x80U | RegDetectionThreshold);
    expect_value(board_write, data, 0x0aU);
    expect_value(board_select, state, false);  
        
    /* RegModemConfig1, RegModemConfig2,  
     * 
     * RegModemConfig1 = Bw (7..6) | CodingRate (5..3) | ImplicitHeader (2) | Crc (1) | LowDateRateOptimise (0)
     * 
     * RegModemConfig2 = SpreadingFactor (7..4) | TxContinuousMode (3) | AgcAutoOn (2) | SymbTimeout (1..0) (MSB)
     * 
     * RegSymbTimeoutLSB (7..0)
     * 
     * RegPreambleMsb
     * RegPreambleLsb
     * 
     * */
    expect_value(board_select, state, true);    
    expect_value(board_write, data, 0x80U | RegModemConfig1);
    expect_value(board_write, data, 0x00U | 0x08U | 0x04U | 0x00U);
    expect_value(board_write, data, 0x70U | 0x04U | 0x00U);
    expect_value(board_write, data, 0x00U);
    expect_value(board_write, data, 0x00U);
    expect_value(board_write, data, 0x08U);
    expect_value(board_select, state, false);  
    
    /* LNA register */
    expect_value(board_select, state, true);    
    expect_value(board_write, data, 0x80U | RegLna);
    expect_value(board_write, data, 0x60U);
    expect_value(board_select, state, false);  
    
    expect_value(board_select, state, true);    
    expect_value(board_write, data, 0x80U | RegOpMode);
    expect_value(board_write, data, 0x86U);
    expect_value(board_select, state, false);  
        
    Radio_receive(&u->radio, &setting);
}

static void test_Radio_sleep(void **user)
{
    struct user_data *u = (struct user_data *)(*user);
    
    expect_value(board_select, state, true);    
    expect_value(board_write, data, 0x80U | RegOpMode);
    expect_value(board_write, data, 0x00U);
    expect_value(board_select, state, false);      
    
    Radio_sleep(&u->radio);
}

static void test_Radio_collect(void **user)
{
    struct user_data *u = (struct user_data *)(*user);
    uint8_t buffer[100];
    
    expect_value(board_select, state, true);    
    expect_value(board_write, data, 0x00U | RegFifoRxCurrentAddr);
    will_return(board_read, 42);
    expect_value(board_select, state, false);      
    
    expect_value(board_select, state, true);    
    expect_value(board_write, data, 0x00U | RegFifo);    
    will_return_count(board_read, 0xa5, 42);    
    expect_value(board_select, state, false);      
    
    assert_int_equal(42, Radio_collect(&u->radio, buffer, sizeof(buffer)));
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_Radio_init, setup_preInit),
        cmocka_unit_test_setup(test_Radio_resetHardware, setup_postInit),
        cmocka_unit_test_setup(test_Radio_transmit, setup_postInit),
        cmocka_unit_test_setup(test_Radio_transmit_noPayload, setup_postInit),
        cmocka_unit_test_setup(test_Radio_sleep, setup_postInit),
        cmocka_unit_test_setup(test_Radio_collect, setup_postInit),
        cmocka_unit_test_setup(test_Radio_receive, setup_postInit),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}

