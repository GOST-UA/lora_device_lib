#include <avr/io.h>
#include <stdint.h>

#ifndef BAUD
    #define BAUD 9600
#endif

void uart_init(void)
{
    UBRR0L = (uint8_t)(F_CPU/(BAUD*16L)-1);
    UCSR0B = (1U<<RXEN0) | (1U<<TXEN0) | (1U<<RXCIE0) | (1U<<TXCIE0);
    UCSR0C = (1U<<UCSZ00) | (1U<<UCSZ01);
}
