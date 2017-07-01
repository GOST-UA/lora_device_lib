LoraDeviceLib
=============

A LoRaWAN device implementation still in an early stage of development.

[![Build Status](https://travis-ci.org/cjhdev/lora_device_lib.svg?branch=master)](https://travis-ci.org/cjhdev/lora_device_lib)

## Highlights

- Region database with optional support for multiple regions at run-time
    - EU_868
- Channel list
    - Statically allocated band and channel structures
    - Power and rate assignment per list
    - Round-Robin channel hopping (managed per band)
    - Channel masking
    - Duty cycle limiter
    - Default channels loaded from region database on initialisation
- Frame codec
    - data
    - join request
    - join accept
    - suitable for device or gateway
- AES and CMAC implementation
- Build-time Porting features
    - Replace AES and CMAC implementations
    - Replace logging and assert functions
    - Optimise chanel capacity / memory usage
- Tests (and testable architecture) utilising the Cmocka framework

## High Level Architecture

LDL uses an OO approach to (try to) separate concerns into modules. Modules are 
as follows:

![image missing](doc/plantuml/modules.png "LoraDeviceLib Modules")

LDL is called from two different tasks - a 'mainloop' level task and an 'interrupt' level task.

The interrupt level task interacts with the EventManager to signal that an IO event has occured. This task will
never block.

The mainloop level task drives all other functionility by calling EventManager.tick():

![image missing](doc/plantuml/event_tick.png "EventManger Tick")

The handlers call out to Mac which may in turn call out to other modules, including back to EventManager
to set handlers for new events or to cancel existing handlers.

It is important to realise that EventManager.tick() drives _all_ functionality. And below a simplified sequence 
of an upstream data frame being sent:

![image missing](doc/plantuml/tick_upstream.png "Upstream")

And below a simplified sequence of a downstream data frame being received:

![image missing](doc/plantuml/tick_downstream.png "Downstream")

More to follow.

## Porting Guide

### Define a Platform Specific Error Message Stream

Define `LORA_ERROR(...)` to be a function that takes a printf style variadic argument.

LoraDeviceLib will print messages to explain why certain functions fail. The
default configuration is to remove these messages from the build.

### Define a Platform Specific Assert Handler

Define `LORA_ASSERT(X)` to be an assertion that takes X as an argument.

LoraDeviceLib makes run time assertions to catch bugs. It is recommended
that this macro is defined at least for debug builds.

### Change the Number of Statically Allocated Band and Channel Structures

- Define `LORA_NUM_CHANNELS` with an integer to define the channel capacity
- Define `LORA_NUM_BANDS` with an integer to define the band capacity

### Substitute an Alternative AES Implementation

1. Define `LORA_USE_PLATFORM_AES`
2. Define `struct lora_aes_ctx` to suit the platform implementation
3. Implement `LoraAES_init` and `LoraAES_encrypt` to wrap platform implementation

### Substitute an Alternative CMAC Implementation

1. Define `LORA_USE_PLATFORM_CMAC`
2. Define `struct lora_cmac_ctx` to suit the platform implementation
3. Implement `LoraCMAC_init`, `LoraCMAC_update`, and `LoraCMAC_finish` to wrap platform implementation

## FAQ

### What is the Difference Between a Channel and a Band?

A channel is a carrier frequency that the radio will transmit/receive on.

A band (or sub-band) is a range of frequencies. Channels fall into
bands depending on region specific rules for how frequency space is
partitioned. A single (logical) band may in some cases be spread over
several non-contiguous frequency partitions.

LoraDeviceLib keeps track of channels and which bands they fall into so as to enforce:

- channel separation and hopping rules
- maximum power rules
- maximum duty cycle rules

## License

LoraDeviceLib has an MIT license.
