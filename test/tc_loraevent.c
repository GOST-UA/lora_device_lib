#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"

#include "lora_event.h"

#include <string.h>

uint32_t getTime(void)
{
    return mock_type(uint32_t);
}

void dummy_handler(void *receiver, uint32_t delay)
{    
    function_called();
    check_expected_ptr(receiver);
    check_expected(delay);
}

static void test_Event_init(void **user)
{
    struct lora_event state;
    
    Event_init(&state);
}

static int setup_Event_init(void **user)
{
    static struct lora_event state;
    Event_init(&state);
    *user = (void *)&state;            
    
    return 0;
}

static void test_Event_onInput_tx_complete(void **user)
{
    struct lora_event *state = (struct lora_event *)(*user);
    
    /* this experiment will call dummy_handler() once */
    expect_function_calls(dummy_handler, 1);
    
    /* register the event */    
    int dummy_receiver = 42;    
    void *event_ptr = Event_onInput(state, EVENT_TX_COMPLETE, &dummy_receiver, dummy_handler);
    
    /* should get a reference back */
    assert_non_null(event_ptr);
    
    /* fire event at time 1 */
    Event_receive(state, EVENT_TX_COMPLETE, 1);
    
    /* if we tick 30 units later delay will be 29 */
    will_return(getTime, 30);
    expect_value(dummy_handler, receiver, (void *)&dummy_receiver);     
    expect_value(dummy_handler, delay, 29);                             
    
    Event_tick(state);
    
    /* fire event at time 40 */
    Event_receive(state, EVENT_TX_COMPLETE, 40);
    
    /* tick again at 60 - no event will be fired */
    will_return(getTime, 60);
    Event_tick(state);    
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_Event_init),
        cmocka_unit_test_setup(test_Event_onInput_tx_complete, setup_Event_init)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
