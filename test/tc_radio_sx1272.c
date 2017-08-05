#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"

#include "lora_radio.h"
#include "lora_regs_sx1272.h"
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
        .erp = DBM_30 
    };
    
    Radio_transmit(&u->radio, &setting, NULL, 0U);
}

static void test_Radio_transmit(void **user)
{
    struct user_data *u = (struct user_data *)(*user);        
    size_t i;
    const uint8_t payload[] = {0x00U, 0x01U, 0x03U};
    
    struct lora_radio_tx_setting setting = {
        .freq = 0xaabbcc,
        .bw = BW_125,
        .sf = SF_7,
        .cr = CR_5,
        .erp = DBM_30 
    };
    
    /* set REG_LR_OPMODE to sleep mode */
    expect_value(board_select, state, true);    
    expect_value(board_write, data, 0x80U | REG_LR_OPMODE);
    expect_value(board_write, data, 0x00U);
    expect_value(board_select, state, false);  
    
    /* set REG_LR_OPMODE to standby mode */
    expect_value(board_select, state, true);    
    expect_value(board_write, data, 0x80U | REG_LR_OPMODE);
    expect_value(board_write, data, 0x81U);
    expect_value(board_select, state, false);  
    
    /* set REG_LR_IRQFLAGS to clear all interrupt flags */
    expect_value(board_select, state, true);    
    expect_value(board_write, data, 0x80U | REG_LR_IRQFLAGS);
    expect_value(board_write, data, 0xffU);
    expect_value(board_select, state, false);  
    
    /* set REG_LR_IRQFLAGSMASK to unmask TX_DONE interrupt */
    expect_value(board_select, state, true);    
    expect_value(board_write, data, 0x80U | REG_LR_IRQFLAGSMASK);
    expect_value(board_write, data, 0xf7U);
    expect_value(board_select, state, false);  
    
    /* set REG_LR_DIOMAPPING1 to raise DIO0 on TX DONE */
    expect_value(board_select, state, true);    
    expect_value(board_write, data, 0x80U | REG_LR_OPMODE);
    expect_value(board_write, data, 1U);
    expect_value(board_select, state, false);  
    
    /* freq */
    expect_value(board_select, state, true);    
    expect_value(board_write, data, 0x80U | REG_LR_FRFMSB);
    expect_value(board_write, data, 0xaaU);
    expect_value(board_write, data, 0xbbU);
    expect_value(board_write, data, 0xccU);
    expect_value(board_select, state, false);  
    
    /* SYNCWORD */
    expect_value(board_select, state, true);    
    expect_value(board_write, data, 0x80U | REG_LR_SYNCWORD);
    expect_value(board_write, data, 0x34U);
    expect_value(board_select, state, false);  
    
    /* REG_LR_MODEMCONFIG1, REG_LR_MODEMCONFIG2,  */
    expect_value(board_select, state, true);    
    expect_value(board_write, data, 0x80U | REG_LR_MODEMCONFIG1);
    expect_value(board_write, data, 0x00U);
    expect_value(board_write, data, 0x00U);
    expect_value(board_write, data, 0x00U);
    expect_value(board_write, data, 0x00U);
    expect_value(board_write, data, 0x08U);
    expect_value(board_select, state, false);  
    
    /* set REG_LR_FIFOADDRPTR to the base address (0) */
    expect_value(board_select, state, true);    
    expect_value(board_write, data, 0x80U | REG_LR_FIFOADDRPTR);
    expect_value(board_write, data, 0x00U);
    expect_value(board_select, state, false);  
    
    /* burst write the FIFO */
    expect_value(board_select, state, true);    
    expect_value(board_write, data, 0x80U | REG_LR_FIFO);    
    for(i=0U; i < sizeof(payload); i++){
        expect_value(board_write, data, payload[i]);
    }        
    expect_value(board_select, state, false);  
    
    /* set REG_LR_OPMODE to single transmit */
    expect_value(board_select, state, true);    
    expect_value(board_write, data, 0x80U | REG_LR_OPMODE);
    expect_value(board_write, data, 0x83U);
    expect_value(board_select, state, false);  
        
    Radio_transmit(&u->radio, &setting, payload, sizeof(payload));
}

static void test_Radio_sleep(void **user)
{
    struct user_data *u = (struct user_data *)(*user);
    
    expect_value(board_select, state, true);    
    expect_value(board_write, data, 0x80U | REG_LR_OPMODE);
    expect_value(board_write, data, 0x00U);
    expect_value(board_select, state, false);      
    
    Radio_sleep(&u->radio);
}

static void test_Radio_collect(void **user)
{
    struct user_data *u = (struct user_data *)(*user);
    uint8_t buffer[100];
    
    expect_value(board_select, state, true);    
    expect_value(board_write, data, 0x00U | REG_LR_FIFORXCURRENTADDR);
    will_return(board_read, 42);
    expect_value(board_select, state, false);      
    
    expect_value(board_select, state, true);    
    expect_value(board_write, data, 0x00U | REG_LR_FIFO);    
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
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}

