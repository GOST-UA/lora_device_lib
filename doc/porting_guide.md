LoraDeviceLib Porting Guide
===========================

This should tell you everything you need to know to port LDL to your
application. 

To port LDL, you must to review each item on the Mandatory list. 

To optimise LDL, you should consider items in the optional list.


## Mandatory

#### Implement System_getDevEUI()

This function must return the DevEUI.

See [lora_system.h](include/lora_system.h).

#### Implement System_getAppEUI()

This function must return the AppEUI

See [lora_system.h](include/lora_system.h).

#### Implement System_getAppKey()

This function must return the AppKey.

See [lora_system.h](include/lora_system.h).

#### Implement System_getTime()

This function must return system time (e.g. time since power up) in microseconds.

See [lora_system.h](include/lora_system.h).

#### Implement System_usleep()

This function must cause the program counter to block for an interval of microseconds.

- LDL may use this for fine timing adjustments where setting a timer event callback
  would take too long

See [lora_system.h](include/lora_system.h).

#### Implement System_atomic_setPtr()

This function must atomically write value to the receiver memory location.

See [lora_system.h](include/lora_system.h).

## Optional

### Specify Region(s) To Support

LDL will by default support all regions. If you are only using
your application in one region this will mean you have some dead code.

Define one or more of the following macros to enable support only
for the regions you want to support:

- `LORA_REGION_EU_863_870`,
- `LORA_REGION_US_902_928`,
- `LORA_REGION_CN_779_787`,
- `LORA_REGION_EU_433`,
- `LORA_REGION_AU_915_928`,
- `LORA_REGION_CN_470_510`,
- `LORA_REGION_AS_923`,
- `LORA_REGION_KR_920_923`

Note that if none of these macros are defined, 
LDL will define `LORA_REGION_ALL` in [lora_region.h](include/lora_region.h).

#### Define LORA_DEVICE

Define this macro to remove code that isn't needed by (most) device implementations.

- Often you only need one half of a codec for a device
- Whole codecs are nice for code reuse and testing

#### Define LORA_DEBUG_INCLUDE

Define this macro to be a filename you want to `#include` at the top of 
[lora_debug.h](include/lora_debug.h).

#### Define LORA_ERROR(...)

Define this macro to be a printf-like function that captures error level messages at run-time.

See [lora_debug.h](include/lora_debug.h).

#### Define LORA_DEBUG(...)

Define this macro to be a printf-like function that captures debug level messages at run-time.

See [lora_debug.h](include/lora_debug.h).

#### Define LORA_INFO(...)

Define this macro to be a printf-like function that captures info level messages at run-time.

See [lora_debug.h](include/lora_debug.h).

#### Define LORA_ASSERT(X)

Define this macro to be an assert-like function that performs run-time asserts on 'X'.

See [lora_debug.h](include/lora_debug.h).

- non-pedantic asserts are useful for development and recommended for production

#### Define LORA_PEDANTIC(X)

Define this macro to be an assert-like function that performs run-time asserts on 'X'.

- pendantic asserts are useful for development but not essential for production

See [lora_debug.h](include/lora_debug.h).

#### Define LORA_AVR

Define this macro to enable AVR specific avr-gcc optimisations.

- Uses PROGMEM where appropriate

#### Substitute an Alternative AES Implementation

1. Define `LORA_USE_PLATFORM_AES`
2. Define `struct lora_aes_ctx` to suit the platform implementation
3. Implement `LoraAES_init` and `LoraAES_encrypt` to wrap platform implementation

See [lora_aes.h](include/lora_aes.h).

#### Substitute an Alternative CMAC Implementation

1. Define `LORA_USE_PLATFORM_CMAC`
2. Define `struct lora_cmac_ctx` to suit the platform implementation
3. Implement `LoraCMAC_init`, `LoraCMAC_update`, and `LoraCMAC_finish` to wrap platform implementation

See [lora_cmac.h](include/lora_cmac.h).

#### Customise lora_radio_sx1272

- define LORA_RADIO_SX1272_USE_BOOST
    - Define this symbol if your hardware makes use of the BOOST pin
    - If not defined, radio will use the RFO pin
        
See [lora_radio_sx1272.c](include/lora_radio_sx1272.h).
