/****
 * Flash detector is TEPT5700 phototransistor biased by a 180K resistor
 * The voltage at emitter is input to pin PD2 and is high when light present
 *
 * Original plan was to create a timer and stop on interrupt using the light
 * sensor to trigger the interrupt. However, given the need only for
 * millisecond accuracy (realistically), chose simply to have a timer
 * interrupt check every millisecond the input status. This seems quite
 * a lot simpler. The AVR 16bit timer has features to time separation
 * of events, but it's generally good for much shorter time spans, I believe,
 * than the 0.1 to many seconds that could occur between electricity meter
 * metrology LED flashes.
 */
#ifndef F_CPU
#define F_CPU 1000000UL
#endif
// timer details
//
#define TICK_INTERVAL 0.001
#define TIMER_PRESCALE 64
// calculate as (F_CPU * TICK_INTERVAL) / TIMER_PRESCALE
// where TICK_INTERVAL is the desired interval. TIMER_COMPARE
// is a byte
#define TIMER_COMPARE 125


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "metroled.h"


volatile uint32_t INTERVAL_MILLIS = 0;
volatile uint32_t LAST_INTERVAL_MILLIS = 0;
volatile uint8_t FLASH_STATUS = 0;


ISR(TIMER1_COMPA_vect)
{
    INTERVAL_MILLIS += 1;
    if (detect_flash() == 1 && FLASH_STATUS == 0) {
        LAST_INTERVAL_MILLIS = INTERVAL_MILLIS;
        INTERVAL_MILLIS = 0;
        FLASH_STATUS = 1;
    }
    else if (detect_flash() == 0 && FLASH_STATUS == 1) {
        FLASH_STATUS = 0;
    }
    return;
}

void init_metrology_led_timer(void) {
    // setup timer - we want a tick every millisecond. Use a 16bit timer
    // to give us accurate intervals.

    // Pin PD2 is input for flash detector
    DDRD &= ~_BV(PIND2);
    // disable pull-up resistor
    PORTD &= ~_BV(PIND2);

    cli();

    TCCR1A = 0;
    TCCR1B = 0;
    // Set compare match value
    OCR1A = TIMER_COMPARE;
    // Enable CTC mode
    TCCR1B |= _BV(WGM12);

    // set prescaler
    switch(TIMER_PRESCALE) {
        case 1:
            TCCR1B |= _BV(CS10);
            break;
        case 8:
            TCCR1B |= _BV(CS11);
            break;
        case 64:
            TCCR1B |= _BV(CS11) | _BV(CS10);
            break;
        case 256:
            TCCR1B |= _BV(CS12);
            break;
        case 1025:
            TCCR1B |= _BV(CS12) | _BV(CS10);
            break;
        default:
            break;
    }

    // Enable timer compare interrupt
    TIMSK |= _BV(OCIE1A);

    // Enable interrupts
    sei();
}


uint8_t detect_flash(void) {
    return (bit_is_set(PIND, 2) > 0) ? 1 : 0;
}


/**
 * if clear == 1, LAST_INTERVAL_MILLIS will be cleared
 * after being read. This allows a calling function to
 * be notified only of new readings.
 */
uint32_t get_metrology_led_interval(uint8_t clear) {
    uint32_t interval = 0;
    cli();
    interval = LAST_INTERVAL_MILLIS;
    if (clear == 1) {
        LAST_INTERVAL_MILLIS = 0;
    }
    sei();
    return interval;
}
