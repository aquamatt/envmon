#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>

int main (void)
{
    // green
    DDRD |= _BV(PIND2);
    // yellow
    DDRD |= _BV(PIND3);

    DDRD &= ~(1 << PIND1);
// enable pull-up resistor
//    PORTD |= 1 << PIND1;
// disable pull-up resistor
    PORTD &= ~_BV(PIND1);

    while(1)
    {
        if (bit_is_clear(PIND, 1))
        {
            PORTD |= _BV(PIND2);
            PORTD &= ~_BV(PIND3);
        } else {
            PORTD &= ~_BV(PIND2);
            PORTD |= _BV(PIND3);
        }
    }
}
