Technical Overview
==================

## Modules

LDL breaks the problem into standalone modules which can be mocked out during testing:

![image missing](doc/plantuml/modules.png "LoraDeviceLib Modules")

LDL is intended to be driven from two tasks - an _interrupt_ level task and a _mainloop_ level task.

The _interrupt_ level task signals when IO events occur. It runs for a short time and never blocks. The _interrupt_ level task must never
be interrupted by the _mainloop_ level task. 

The _mainloop_ level task drives all other functionility from a single thread of execution. 
There are a small number of critical sections between the _interrupt_ level and _mainloop_ level threads - the _mainloop_ level task
will always use atomic operations when accessing these sections.

All _mainloop_ code runs from calls to EventManager.tick():

![image missing](doc/plantuml/event_tick.png "EventManger Tick")

Below is a simplified sequence of sending data:

![image missing](doc/plantuml/tick_upstream.png "Upstream")

Below is a simplified sequence of recieving data:

![image missing](doc/plantuml/tick_downstream.png "Downstream")

## Timing Requirements

LoRaWAN requires that receive windows open at a precise interval measured
from the end of the TX. This can be challenging to implement on a busy system
that doesn't use an RTOS since other tasks in the mainloop will add
jitter to when LDL can open the RX window.

The diagram below illustrates the TX -> RX1 -> RX2 pattern:

![image missing](doc/plantuml/rx_windows.png "RX Timing")

The architecture handles RX windows as follows:

- The end of TX is detected by an IO event sent from the transeiver to the EventManager via an interrupt
    - The EventManager keeps a microsecond timestamp of when this IO event is received
- The IO event is handled by EventManager.tick() at any time after TX and before the RX window
    - This is a one or more second window
- The IO event handler schedules a timer callback to run at the RX window time
    - The interval is calculated as `interval = RX_INTERVAL - (time_now - tx_end_time) - spi_delay - typical_jitter`
    - `spi_delay` is time added to account for communication between LDL and the radio transciever (i.e. over SPI)
    - `typical_jitter` is time added to account for jitter in handling the timeout event
- The timeout event handler runs after the aforementioned interval
    - The handler calculates the current time relative to the RX window
        - if late, LDL abandons the RX window        
        - if early, LDL blocks until the start of the RX window
- This pattern is the same for RX1 and RX2 windows
    
In summary:

- In LDL, IO events are lower priority than timeout events, and timeout events work best when jitter is low
- RX1 and RX2 windows are met on best effort
    - Timeout events can be set to trigger early to account for worst case jitter
    - LDL will skip an RX window if the timestamps show that the timeout has been handled too late
- The effect of jitter on LDL is that:
    - Clock cycles may be wasted blocking
    - RX windows may be missed

## Channel Schedule / Hopping Algorithm

### Data

LDL implements the following algorithm for data messages:

- Let `B` be a set of all bands for the current region.
- Let `b` be a subset of B where the band is ready to be used: 
- Let `f` be a set of frequencies that belong to bands in `b` which are not masked and which support the current datarate setting:
- Let `s` be a randomly selected frequency from `f` 

`s` shall be the channel used for upstream transmission. 

