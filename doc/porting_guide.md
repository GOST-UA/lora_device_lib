LoraDeviceLib Porting Guide
===========================

## Mandatory

### Implement All [lora_system.h](/include/lora_system.h) Interfaces

Most of these interfaces exist to give the integrator a choice in how
data is stored on their target (i.e. in volatile or non-volatile memory).

## Optional

### Specify Region(s) To Support

LDL will by default support all regions. If your device is only meant
to work in one region, you can use this macro to remove code that
isn't required for your region.

- `LORA_REGION_EU_863_870`,
- `LORA_REGION_US_902_928`,
- `LORA_REGION_CN_779_787`,
- `LORA_REGION_EU_433`,
- `LORA_REGION_AU_915_928`,
- `LORA_REGION_CN_470_510`,
- `LORA_REGION_AS_923`,
- `LORA_REGION_KR_920_923`

Note that you can define one or more of these macros. If you don't define
any of these macros, LDL will define `LORA_REGION_ALL`.

See [lora_region.h](/include/lora_region.h) for more information.

### Define LORA_DEVICE

LDL implements complete frame and message codecs. This means that LDL has code 
to encode or decode messages that a Device would not normally need to encode or decode.

Define `LORA_DEVICE` to ensure only the codec functions needed for a Device are included
in your build.

### Define LORA_DEBUG_INCLUDE

Define `LORA_DEBUG_INCLUDE` to be a filename that you want to include at the top of
[lora_debug.h](/include/lora_debug.h).

This file can contain anything you like. Normally it will contain any definitions
required for your customised debug macros 
(i.e. `LORA_ERROR`, `LORA_DEBUG`, `LORA_INFO`, `LORA_ASSERT`, and `LORA_PEDANTIC`).

### Define LORA_ERROR(...)

Define this macro to be a printf-like function that captures error level messages at run-time.
If not defined, all LORA_ERROR messages will be left out of the build.

See [lora_debug.h](/include/lora_debug.h).

### Define LORA_DEBUG(...)

Define this macro to be a printf-like function that captures debug level messages at run-time.
If not defined, all LORA_DEBUG messages will be left out of the build.

See [lora_debug.h](/include/lora_debug.h).

### Define LORA_INFO(...)

Define this macro to be a printf-like function that captures info level messages at run-time.
If not defined, all LORA_INFO messages will be left out of the build.

See [lora_debug.h](/include/lora_debug.h).

### Define LORA_ASSERT(X)

Define this macro to be an assert-like function that performs run-time asserts on 'X'.
If not defined, all LORA_ASSERT assertions will be left out of the build.

See [lora_debug.h](/include/lora_debug.h).

### Define LORA_PEDANTIC(X)

Define this macro to be an assert-like function that performs run-time asserts on 'X'.
These asserts are considered pedantic and not essential for production.
If not defined, all LORA_PEDANTIC assertions will be left out of the build.

See [lora_debug.h](/include/lora_debug.h).

### Define LORA_AVR

Define this macro to enable AVR specific avr-gcc optimisations.

- Uses PROGMEM where appropriate

### Substitute an Alternative AES Implementation

1. Define `LORA_USE_PLATFORM_AES`
2. Define `struct lora_aes_ctx` to suit the platform implementation
3. Implement `LoraAES_init` and `LoraAES_encrypt` to wrap platform implementation

See [lora_aes.h](/include/lora_aes.h).

### Substitute an Alternative CMAC Implementation

1. Define `LORA_USE_PLATFORM_CMAC`
2. Define `struct lora_cmac_ctx` to suit the platform implementation
3. Implement `LoraCMAC_init`, `LoraCMAC_update`, and `LoraCMAC_finish` to wrap platform implementation

See [lora_cmac.h](/include/lora_cmac.h).

### Customise lora_radio_sx1272

- define LORA_RADIO_SX1272_USE_BOOST
    - Define this symbol if your hardware makes use of the BOOST pin
    - If not defined, radio will use the RFO pin
        
See [lora_radio_sx1272.c](/include/lora_radio_sx1272.h).
