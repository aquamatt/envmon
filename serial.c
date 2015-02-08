/**********************************
 * Serial comms funcions
 */
#ifndef F_CPU
#define F_CPU 1000000UL
#endif
#define BAUDRATE ((F_CPU)/(BAUD*16UL)-1)

#include <stdio.h>
#include <avr/io.h>
#include "serial.h"

void uart_init(void) {
    UBRRH = BAUDRATE >> 8;
    UBRRL = BAUDRATE;

    UCSRC = _BV(URSEL) |_BV(UCSZ1) | _BV(UCSZ0); /* 8-bit data */
    UCSRB = _BV(RXEN) | _BV(TXEN);   /* Enable RX and TX */
}

void uart_putchar(char c, FILE *stream) {
    if (c == '\n') {
        uart_putchar('\r', stream);
    }
    while (!( UCSRA & _BV(UDRE)));  /* Wait until data register empty. */
    UDR = c;
}
FILE uart_output = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

void redirect_stdout(void) {
    stdout = &uart_output;
}
