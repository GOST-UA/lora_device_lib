#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"

#include "lora_event.h"
#include "lora_system.h"

#include <string.h>

#include "mock_system_time.h"

/* helpers */

static void eventHandler(void *receiver, uint64_t time, uint64_t error)
{    
    check_expected_ptr(receiver);
    check_expected_ptr(time);
    check_expected(error);
}

/* setups */

static int setup_event(void **user)
{
    static struct lora_event state;
    Event_init(&state);
    *user = (void *)&state;                
    system_time = 0U;
    return 0;
}

static int setup_event_and_register_timeout_at_42_ticks(void **user)
{
    int retval = setup_event(user);
    
    if(retval == 0U){
        
        struct lora_event *self = (struct lora_event *)(*user);    
                
        if(Event_onTimeout(self, 42U, self, eventHandler) == NULL){
            
            retval = -1;
        }
    }
    
    return retval;    
}

#if 0

static int setup_event_and_register_and_answer_onInput(void **user)
{
    int retval = setup_event(user);
    
    if(retval == 0U){
        
        system_time = 42U;
        
        struct lora_event *self = (struct lora_event *)(*user);    
        
        Event_onInput(self, EVENT_TX_COMPLETE, self, eventHandler);
        
        Event_receive(self, EVENT_TX_COMPLETE, System_time());
    }
    
    return retval;
}

static int setup_event_and_recieve_input(void **user)
{
    int retval = setup_event(user);
    
    if(retval == 0U){
        
        system_time = 42U;
        
        struct lora_event *self = (struct lora_event *)(*user);    
        
        Event_receive(self, EVENT_TX_COMPLETE, System_time());
    }
    
    return retval;
}

#endif

/* expectations */

static void event_shall_be_initialised(void **user)
{
    assert_int_equal(0, setup_event(user));    
}

static void onTimeout_shall_register_timout_handler(void **user)
{    
    struct lora_event *self = (struct lora_event *)(*user);    
    assert_true( Event_onTimeout(self, 0U, self, eventHandler) != NULL );    
}

static void onTimeout_shall_return_null_if_resource_not_available(void **user)
{    
    struct lora_event *self = (struct lora_event *)(*user);    
    size_t i;
    
    for(i=0U; i < EVENT_NUM_TIMERS; i++){
        
        Event_onTimeout(self, 0U, self, eventHandler);
    }
    
    assert_true( Event_onTimeout(self, 0U, self, eventHandler) == NULL );    
}

static void tick_shall_service_handler_at_timeout(void **user)
{
    struct lora_event *self = (struct lora_event *)(*user);    
    
    system_time = 42U;
    
    expect_value(eventHandler, receiver, self);
    expect_value(eventHandler, time, system_time);
    expect_value(eventHandler, error, 0U);
    
    Event_tick(self);    
}

static void tick_shall_service_handler_after_timeout(void **user)
{
    struct lora_event *self = (struct lora_event *)(*user);    
    
    system_time = 43U;
    
    expect_value(eventHandler, receiver, self);
    expect_value(eventHandler, time, system_time);
    expect_value(eventHandler, error, 1U);
    
    Event_tick(self);    
}

static void tick_shall_not_service_handler_before_timeout(void **user)
{
    struct lora_event *self = (struct lora_event *)(*user);    
    
    Event_tick(self);    
}

static void intervalUntilNext_shall_return_max_interval_if_no_event_is_pending(void **user)
{
    struct lora_event *self = (struct lora_event *)(*user);    
    
    assert_true( Event_intervalUntilNext(self) == UINT64_MAX );
}

static void intervalUntilNext_shall_return_interval_until_next_pending_event(void **user)
{
    struct lora_event *self = (struct lora_event *)(*user);    
    
    assert_true( Event_intervalUntilNext(self) == 42U );
    
    system_time = 40U;
    
    assert_true( Event_intervalUntilNext(self) == 2U );
    
    system_time = 50U;
    
    assert_true( Event_intervalUntilNext(self) == 0U );
}

static void cancel_shall_remove_timeout_handler(void **user)
{
    struct lora_event *self = (struct lora_event *)(*user);    
    
    void *ptr = Event_onTimeout(self, 42U, self, eventHandler);
    
    assert_true( ptr != NULL );
    assert_true( Event_intervalUntilNext(self) == 42U );
    
    Event_cancel(self, &ptr);
    
    assert_true( ptr == NULL );
    assert_true( Event_intervalUntilNext(self) == UINT64_MAX );    
}

static void cancel_shall_remove_input_handler(void **user)
{
    struct lora_event *self = (struct lora_event *)(*user);    
    
    void *ptr = Event_onInput(self, 42U, self, eventHandler);    
    Event_cancel(self, &ptr);    
}

static void onInput_shall_register_input_handler(void **user)
{    
    struct lora_event *self = (struct lora_event *)(*user);    
    
    assert_true( Event_onInput(self, EVENT_TX_COMPLETE, self, eventHandler) );
}

/* runner */

int main(void)
{
    const struct CMUnitTest tests[] = {
        
        cmocka_unit_test(
            event_shall_be_initialised
        ),
        
        cmocka_unit_test_setup(
            onTimeout_shall_register_timout_handler, 
            setup_event
        ),
        cmocka_unit_test_setup(
            onTimeout_shall_return_null_if_resource_not_available,
            setup_event
        ),
        
        cmocka_unit_test_setup(
            tick_shall_service_handler_at_timeout, 
            setup_event_and_register_timeout_at_42_ticks
        ),
        cmocka_unit_test_setup(
            tick_shall_service_handler_after_timeout, 
            setup_event_and_register_timeout_at_42_ticks
        ),
        cmocka_unit_test_setup(
            tick_shall_not_service_handler_before_timeout, 
            setup_event_and_register_timeout_at_42_ticks
        ),
        
        cmocka_unit_test_setup(
            intervalUntilNext_shall_return_max_interval_if_no_event_is_pending, 
            setup_event
        ),
        cmocka_unit_test_setup(
            intervalUntilNext_shall_return_interval_until_next_pending_event, 
            setup_event_and_register_timeout_at_42_ticks
        ),
        
        cmocka_unit_test_setup(
            onInput_shall_register_input_handler, 
            setup_event
        ),            
        
        cmocka_unit_test_setup(
            cancel_shall_remove_timeout_handler, 
            setup_event
        ),        
        cmocka_unit_test_setup(
            cancel_shall_remove_input_handler, 
            setup_event
        ),        
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
