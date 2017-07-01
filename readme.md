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

### Define a Platform Specific Error Message Stream (Optional)

Define `LORA_ERROR(...)` to be a function that takes a printf style variadic argument.

LoraDeviceLib will print messages to explain why certain functions fail. The
default behaviour is to not do anything with these messsages.

### Define a Platform Specific Assert Handler (Optional but Recommended)

Define `LORA_ASSERT(X)` to be an assertion that takes X as an argument.

LoraDeviceLib makes run time assertions to catch bugs. It is recommended
that this macro is defined at least for debug builds.

### Change the Number of Statically Allocated Band and Channel Structures (Optional)

- Define `LORA_NUM_CHANNELS` with an integer to define the channel capacity
- Define `LORA_NUM_BANDS` with an integer to define the band capacity

On a constrained platform is possible to save memory by reducing the channel/band
capacity.

### Substitute an Alternative AES Implementation (Optional)

1. Define `LORA_USE_PLATFORM_AES`
2. Define `struct lora_aes_ctx` to suit the platform implementation
3. Implement `LoraAES_init` and `LoraAES_encrypt` to wrap platform implementation

Perform these steps if you want to use a platform specific implementation.
This is definitely a good idea
if your platform AES implementation has some kind of certification and/or has
seen extensive use.

### Substitute an Alternative CMAC Implementation (optional)

1. Define `LORA_USE_PLATFORM_CMAC`
2. Define `struct lora_cmac_ctx` to suit the platform implementation
3. Implement `LoraCMAC_init`, `LoraCMAC_update`, and `LoraCMAC_finish` to wrap platform implementation

Perform these steps if you want to use a platform specific implementation.
This is definitely a good idea
if your platform AES implementation has some kind of certification and/or has
seen extensive use.

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

### What Are All the Build Options?

~~~
# Removes the LoraDeviceLib CMAC implementation from the build
# default: not-defined
-DLORA_USE_PLATFORM_CMAC

# Removes the LoraDeviceLib AES implementation from the build
# default: not-defined
-DLORA_USE_PLATFORM_AES

# Define the channel capacity of channel list
# default: 16
-DLORA_NUM_CHANNELS=16U

# Define the band capacity of channel list
# default: 5
-DLORA_NUM_BANDS=5U

# Define LORA_ERROR()
# default: empty
-DLORA_ERROR(X) ;

# Define LORA_ASSERT()
# default: empty
-DLORA_ASSERT(X) ;
~~~

## License

LoraDeviceLib has an MIT license.
