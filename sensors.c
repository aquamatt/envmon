/******
 * Build notes
 * - F_CPU should probably be overridden in Makefile
 *
 * Other notes
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

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "i2c/i2cmaster.h"
#include "mydisplay.h"
#include "bmp180.h"
#include "serial.h"

// timer details
//
#define TICK_INTERVAL 0.001
#define TIMER_PRESCALE 64
// calculate as (F_CPU * TICK_INTERVAL) / TIMER_PRESCALE
// where TICK_INTERVAL is the desired interval. TIMER_COMPARE
// is a byte
#define TIMER_COMPARE 125

volatile uint32_t INTERVAL_MILLIS = 0;
volatile uint32_t LAST_INTERVAL_MILLIS = 0;
volatile uint8_t BUFFER_OVERFLOW = 0;
volatile uint8_t FLASH_STATUS = 0;

void verify_bmp(void) {
    uint8_t status = 0;
    status = initialise_bmp_module();
    if (status != 0)
    {
        while(1)
        {
            display(status);
            _delay_ms(500);
            display(0);
            _delay_ms(500);
        }
    }
}

void init_flash_timer(void) {
    // setup timer - we want a tick every millisecond. Use a 16bit timer
    // to give us accurate intervals.

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

ISR(TIMER1_COMPA_vect)
{
    INTERVAL_MILLIS += 1;
    if (detect_flash() == 1 && FLASH_STATUS == 0) {
        cli();
        if (LAST_INTERVAL_MILLIS > 0)
        {
            BUFFER_OVERFLOW = 1;
        }
        LAST_INTERVAL_MILLIS = INTERVAL_MILLIS;
        INTERVAL_MILLIS = 0;
        sei();
        FLASH_STATUS = 1;
    }
    else if (detect_flash() == 0 && FLASH_STATUS == 1) {
        FLASH_STATUS = 0;
    }
}

int main (void)
{
    // output LED configuration
    DDRB = 0b00011111;
    // Pin PD2 is input for flash detector
    DDRD &= ~_BV(PIND2);
    // disable pull-up resistor
    PORTD &= ~_BV(PIND2);

    i2c_init();
    uart_init();
    redirect_stdout();
    verify_bmp();
    init_flash_timer();

    uint32_t temperature = 0;
    uint32_t pressure = 0;
    uint32_t interval = 0;
    uint16_t loop_count = 0;

    /***
     * A deliberate decision was taken not to have the photo transistor
     * trigger an interrupt, but to have the 16bit timer trigger interrupts
     * at 1ms intervals and test the status of the photo transistor input
     * pin. This loop then checks for an update (LAST_INTERVAL_MILLIS non
     * zero indicates a pulse time measurement) and sends it to the UART.
     * It also checks for the badly named BUFFER_OVERFLOW condition set which
     * would indicate that a second flash had been detected prior to the
     * preceding one being processed.
     *
     * Once in every 500 loops (approx 5s given the 10ms delay +
     * any processing done) temperature and pressure are read. Subsequent
     * if statements push to the UART when a reading is present.
     */
    while(1){
        loop_count += 1;
        if (loop_count == 500) {
            // only measure once every 5 seconds or so
            temperature = get_temp();
            //pressure = get_pressure(ULTRALOWPOWER);
            pressure = get_pressure(ULTRAHIGHRES);
            loop_count = 0;
        }
        if (temperature != 0)
        {
            printf("T=%ld\n", temperature);
            display((uint8_t)(((float)temperature/10.0)+0.5));
            temperature = 0;
        }
        if (pressure != 0)
        {
            printf("P=%ld\n", pressure);
            pressure = 0;
        }
        if (LAST_INTERVAL_MILLIS != 0) {
            cli();
            interval = LAST_INTERVAL_MILLIS;
            LAST_INTERVAL_MILLIS = 0;
            sei();
            if (BUFFER_OVERFLOW == 1) {
                BUFFER_OVERFLOW = 0;
                printf("BO=1\n");
            }
            printf("I=%ld\n", interval);
        }
        _delay_ms(10);
    }
}
