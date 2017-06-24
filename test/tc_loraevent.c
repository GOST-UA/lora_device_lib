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

void handler(void *receiver, uint32_t delay)
{    
    
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

static void test_Event_onInput(void **user)
{
    struct lora_event *state = (struct lora_event *)(*user);
    
    int dummy_receiver = 42;
    
    void *event_ptr = Event_onInput(state, EVENT_TX_COMPLETE, &dummy_receiver, handler);
    
    assert_non_null(event_ptr);
    
    will_return(getTime, 0);
    
    Event_tick(state);
    
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_Event_init),
        cmocka_unit_test_setup(test_Event_onInput, setup_Event_init)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
