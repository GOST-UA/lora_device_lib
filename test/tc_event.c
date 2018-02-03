#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"

#include "lora_event.h"

#include <string.h>

uint64_t System_getTime(void)
{
    return mock();
}

void dummy_handler(void *receiver, uint64_t time)
{    
    function_called();
    check_expected_ptr(receiver);
    check_expected(time);
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
    
    /* tick 30 time units later */
    will_return(System_getTime, 30);
    expect_value(dummy_handler, receiver, (void *)&dummy_receiver);     
    expect_value(dummy_handler, time, 1);       // event occured at time 1                             
    
    Event_tick(state);
    
    /* fire event at time 40 */
    Event_receive(state, EVENT_TX_COMPLETE, 40);
    
    /* tick again at 60 - no event will be fired */
    will_return(System_getTime, 60);
    Event_tick(state);    
}

static void test_Event_onInput_tx_complete_cancel(void **user)
{
    struct lora_event *state = (struct lora_event *)(*user);
    
    /* can't specify zero function calls */
    ignore_function_calls(dummy_handler);
    
    /* register the event */    
    int dummy_receiver = 42;    
    void *event_ptr = Event_onInput(state, EVENT_TX_COMPLETE, &dummy_receiver, dummy_handler);
    
    /* should get a reference back */
    assert_non_null(event_ptr);
    
    /* let's cancel it */
    Event_cancel(state, &event_ptr);
    
    assert_null(event_ptr);
    
    /* fire event at time 1 */
    Event_receive(state, EVENT_TX_COMPLETE, 1);
    
    /* if we tick 30 units later delay will be 29 */
    will_return(System_getTime, 30);

    Event_tick(state);
}

static void test_Event_onTimeout(void **user)
{
    struct lora_event *state = (struct lora_event *)(*user);
    
    /* this experiment will call dummy_handler() once */
    expect_function_calls(dummy_handler, 1);
    
    /* register the event to timeout in 25 units from now (1) */    
    int dummy_receiver = 42;    
    void *event_ptr = Event_onTimeout(state, 25, &dummy_receiver, dummy_handler);
    
    /* should get a reference back */
    assert_non_null(event_ptr);
    
    /* tick at 30 time units */
    will_return(System_getTime, 30);
    expect_value(dummy_handler, receiver, (void *)&dummy_receiver);     
    expect_value(dummy_handler, time, 25);                                 
    Event_tick(state);
    
    /* tick again without advancing time...dummy_handler should not be called */
    will_return(System_getTime, 30);
    Event_tick(state);
}

static void test_Event_onTimeout_cancel(void **user)
{
    struct lora_event *state = (struct lora_event *)(*user);
    
    /* can't specify zero calls so disable call counter - parameter check will ensure function wont be called */
    ignore_function_calls(dummy_handler);
    
    /* register the event to timeout in 25 units from now (1) */    
    int dummy_receiver = 42;    
    
    void *event_ptr = Event_onTimeout(state, 25, &dummy_receiver, dummy_handler);
    
    /* should get a reference back */
    assert_non_null(event_ptr);
    
    /* let's cancel it */
    Event_cancel(state, &event_ptr);
    
    assert_null(event_ptr);
    
    /* if we tick 30 units later delay will be 4 ... but the event will not be handled since it cancelled */
    will_return(System_getTime, 30);
    Event_tick(state);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_Event_init),
        cmocka_unit_test_setup(test_Event_onInput_tx_complete, setup_Event_init),
        cmocka_unit_test_setup(test_Event_onTimeout, setup_Event_init),
        cmocka_unit_test_setup(test_Event_onTimeout_cancel, setup_Event_init),
        cmocka_unit_test_setup(test_Event_onInput_tx_complete_cancel, setup_Event_init),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
