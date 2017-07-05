#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include "cmocka.h"

#include "lora_mac.h"

#include <string.h>

struct lora_channel_list {
    
};

struct lora_radio {
        
};

struct lora_event {
    
};

enum lora_region_id ChannelList_region(const struct lora_channeL_list *self)
{
    return EU_863_870;
}


bool LoraRadio_setParameters(struct lora_radio *self, uint32_t freq, enum signal_bandwidth bw, enum spreading_factor sf)
{
    return false;
}

void LoraRadio_transmit(struct lora_radio *self, const void *data, uint8_t len)
{
}

void LoraRadio_receive(struct lora_radio *self)
{    
}

uint8_t LoraRadio_collect(struct lora_radio *self, void *data, uint8_t max)
{
    return 0;
}

void ChannelList_registerTransmission(struct lora_channel_list *self, uint32_t timeNow, uint8_t payloadLen)
{
}

uint64_t getTime(void)
{
    return 0U;
}

uint32_t ChannelList_waitTime(const struct lora_channel_list *self, uint64_t timeNow)
{
    return 0U;
}


static int setup_LoraMAC(void **user)
{
    static struct lora_mac self;
    static struct lora_channel_list channels;
    static struct lora_radio radio;
    static struct lora_event events;    
    
    LoraMAC_init(&self, &channels, &radio, &events);
    
    *user = (void *)&self;
    
    return 0;
}

static void test_LoraMAC_setSession(void **user)
{
    struct lora_mac *self = (struct lora_mac *)(*user);
    
    LoraMAC_setSession(self, 1, "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa", "\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb\xbb");
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup(test_LoraMAC_setSession, setup_LoraMAC),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}


