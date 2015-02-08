#include <avr/io.h>
#include <util/delay.h>
#include "mydisplay.h"

/***********************
 * Display to our 5-bit output
 */
void display(uint8_t v) {
   PORTB &= ~LOWER_FIVE;
   v &= LOWER_FIVE;
   PORTB |= v;
}

void displayf(float v) {
   display((uint8_t)v);
}

void flash(uint8_t v, uint8_t count) {
    uint8_t i;
    for (i=0; i<count; i++)
    {
        display(v);
        _delay_ms(500);
        display(0);
        _delay_ms(500);
    }
    display(v);
}

