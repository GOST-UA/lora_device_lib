#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"

#include "lora_mac.h"
#include "lora_radio_sx1272.h"
#include "lora_frame.h"

#include "mock_lora_system.h"
#include "mock_system_time.h"

#include <string.h>

static const uint8_t eui[] = "\x00\x00\x00\x00\x00\x00\x00\x00";
static const uint8_t key[] = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

static void responseHandler(void *receiver, enum lora_mac_response_type type, const union lora_mac_response_arg *arg);
static bool future_event_is_pending(struct lora_mac *self);
static bool immediate_event_is_pending(struct lora_mac *self);

/* helpers */

static void responseHandler(void *receiver, enum lora_mac_response_type type, const union lora_mac_response_arg *arg)
{
    enum lora_mac_response_type expected_type = mock();
    
    assert_true(type == expected_type);    
}

static bool future_event_is_pending(struct lora_mac *self)
{
    return (MAC_intervalUntilNext(self) > 0U) && (MAC_intervalUntilNext(self) != UINT64_MAX);
}

static bool immediate_event_is_pending(struct lora_mac *self)
{
    return (MAC_intervalUntilNext(self) == 0U);
}

/* setup */

static int setup_mac(void **user)
{
    static struct lora_mac self;
    static struct lora_radio radio;
    static struct mock_system_param params;
        
    system_time = 0U;
        
    mock_lora_system_init(&params);
    
    MAC_init(&self, &params, EU_863_870, &radio);
    
    MAC_restoreDefaults(&self);
    
    *user = (void *)&self;
    
    return 0;
}

/* expectations */

static void init_shall_initialise(void **user)
{
    assert_true( setup_mac(user) == 0 );
}

static void personalize_shall_perform_abp(void **user)
{
    struct lora_mac *self = (struct lora_mac *)(*user);
    uint32_t devAddr = 0U;
    
    bool retval = MAC_personalize(self, devAddr, "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa", "\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb");
    
    assert_true(retval);
}

static void join_shall_timeout(void **user)
{
    struct lora_mac *self = (struct lora_mac *)(*user);
    
    MAC_setResponseHandler(self, NULL, responseHandler);
    
    // initiate join
    assert_true(MAC_join(self));
    assert_true(immediate_event_is_pending(self));
    
    // next tick will put radio into TX mode
    will_return(Radio_transmit, true);    
    MAC_tick(self);   
    
    // io event: tx complete
    MAC_radioEvent(self, LORA_RADIO_TX_COMPLETE, System_getTime());
    assert_true(immediate_event_is_pending(self));
    MAC_tick(self);
    assert_true(future_event_is_pending(self));
    
    // advance time to T_RX1
    system_time += MAC_intervalUntilNext(self);
    
    // next tick will put radio into RX mode
    will_return(Radio_receive, true);    
    MAC_tick(self);
    
    // io event: rx_timeout
    MAC_radioEvent(self, LORA_RADIO_RX_TIMEOUT, System_getTime());
    assert_true(immediate_event_is_pending(self));
    MAC_tick(self);
    assert_true(future_event_is_pending(self));
    
    // advance time to T_RX2
    system_time += MAC_intervalUntilNext(self);
    
    // next tick will put radio into RX mode
    will_return(Radio_receive, true);    
    MAC_tick(self);
    
    // io event: rx_timeout
    MAC_radioEvent(self, LORA_RADIO_RX_TIMEOUT, System_getTime());
    assert_true(immediate_event_is_pending(self));    
    
    // next tick shall yield a callback to responseHandler
    will_return(responseHandler, LORA_MAC_JOIN_TIMEOUT);
    MAC_tick(self);    
}

static void join_shall_succeed_at_rx1(void **user)
{
    struct lora_mac *self = (struct lora_mac *)(*user);
    size_t messageSize;
    uint8_t message[50U];
    struct lora_frame_join_accept ja;
    
    // prepare join accept    
    (void)memset(&ja, 0, sizeof(ja));
    messageSize = Frame_putJoinAccept(key, &ja, message, sizeof(message));    
        
    MAC_setResponseHandler(self, NULL, responseHandler);
    
    // initiate join
    assert_true(MAC_join(self));
    assert_true(immediate_event_is_pending(self));
    
    // next tick will put radio into TX mode
    will_return(Radio_transmit, true);    
    MAC_tick(self);   
    
    // io event: tx complete
    MAC_radioEvent(self, LORA_RADIO_TX_COMPLETE, System_getTime());
    assert_true(immediate_event_is_pending(self));
    MAC_tick(self);
    assert_true(future_event_is_pending(self));
    
    // advance time to T(rx1)
    system_time += MAC_intervalUntilNext(self);
    
    // next tick will put radio into RX mode
    will_return(Radio_receive, true);    
    MAC_tick(self);
    
    // io event: rx_ready
    will_return(Radio_collect, (uint8_t)messageSize);
    will_return(Radio_collect, message);
    MAC_radioEvent(self, LORA_RADIO_RX_READY, System_getTime());
    assert_true(immediate_event_is_pending(self));
    MAC_tick(self);
    assert_true(future_event_is_pending(self));
    
    // advance time to T(rx2)
    system_time += MAC_intervalUntilNext(self);
    
    // next tick will put radio into RX mode
    will_return(Radio_receive, true);    
    MAC_tick(self);
    
    // io event: rx_timeout
    MAC_radioEvent(self, LORA_RADIO_RX_TIMEOUT, System_getTime());
    assert_true(immediate_event_is_pending(self));    
    
    // next tick shall yield a callback to responseHandler
    will_return(responseHandler, LORA_MAC_JOIN_SUCCESS);
    MAC_tick(self);    
}

static void join_shall_succeed_at_rx2(void **user)
{
    struct lora_mac *self = (struct lora_mac *)(*user);
    size_t messageSize;
    uint8_t message[50U];
    struct lora_frame_join_accept ja;
    
    // prepare join accept    
    (void)memset(&ja, 0, sizeof(ja));
    messageSize = Frame_putJoinAccept(key, &ja, message, sizeof(message));    
        
    MAC_setResponseHandler(self, NULL, responseHandler);
    
    // initiate join
    assert_true(MAC_join(self));
    assert_true(immediate_event_is_pending(self));
    
    // next tick will put radio into TX mode
    will_return(Radio_transmit, true);    
    MAC_tick(self);   
    
    // io event: tx complete
    MAC_radioEvent(self, LORA_RADIO_TX_COMPLETE, System_getTime());
    assert_true(immediate_event_is_pending(self));
    MAC_tick(self);
    
    // advance time to T(rx1)
    system_time += MAC_intervalUntilNext(self);
    
    // next tick will put radio into RX mode
    will_return(Radio_receive, true);    
    MAC_tick(self);
    
    // io event: rx_timeout
    MAC_radioEvent(self, LORA_RADIO_RX_TIMEOUT, System_getTime());
    assert_true(immediate_event_is_pending(self));
    MAC_tick(self);
    
    // advance time to T(rx2)
    system_time += MAC_intervalUntilNext(self);
    
    // next tick will put radio into RX mode
    will_return(Radio_receive, true);    
    MAC_tick(self);
    
    // io event: rx_ready
    will_return(Radio_collect, (uint8_t)messageSize);
    will_return(Radio_collect, message);
    MAC_radioEvent(self, LORA_RADIO_RX_READY, System_getTime());
    assert_true(immediate_event_is_pending(self));    
    
    // next tick shall yield a callback to responseHandler
    will_return(responseHandler, LORA_MAC_JOIN_SUCCESS);
    MAC_tick(self);    
}

/* runner */

int main(void)
{
    const struct CMUnitTest tests[] = {
        
        cmocka_unit_test(
            init_shall_initialise
        ),
        
        cmocka_unit_test_setup(
            personalize_shall_perform_abp, 
            setup_mac
        ),
        
        cmocka_unit_test_setup(
            join_shall_timeout, 
            setup_mac
        ),
        
        cmocka_unit_test_setup(
            join_shall_succeed_at_rx1, 
            setup_mac
        ),
        
        cmocka_unit_test_setup(
            join_shall_succeed_at_rx2, 
            setup_mac
        )        
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}



