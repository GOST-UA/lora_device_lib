LoraDeviceLib
=============

A LoRaWAN device implementation

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
- Frame encode/decode functions with integrated cryptography
- Tests

## Porting Guide

### Define a Platform Specific Error Message Stream

Define `LORA_ERROR(...)` to be a function that takes a printf style variadic argument.

LoraDeviceLib will print messages to explain why certain functions fail. The
default behaviour is to remove these messages from the build.

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
