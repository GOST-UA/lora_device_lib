#include <avr/interrupt.h>
#include <util/atomic.h>
#include <avr/io.h>
#include <avr/wdt.h>

#include <stddef.h>

#include "ldl.h"
#include "uart.h"

void main(void) __attribute__((noreturn));



/* gets called when ldl is ready to transmit */
static void app(void **state)
{
}

void main(void)
{
    uart_init();
    
    ldl_init();
    
    while(true){
        
        ldl_tick(NULL, app);            
    }
}

