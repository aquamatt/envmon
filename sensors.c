/******
 * Build notes
 * - F_CPU should probably be overridden in Makefile
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
#include "metroled.h"
#include "serial.h"


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


int main(void) {
    mydisplay_init();
    i2c_init();
    uart_init();
    redirect_stdout();
    verify_bmp();
    init_metrology_led_timer();

    uint32_t temperature = 0;
    uint32_t pressure = 0;
    uint32_t interval = 0;
    uint16_t loop_count = 499; // force immediate initial measure

    /***
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

        interval = get_metrology_led_interval(1);
        if (interval != 0) {
            printf("I=%ld\n", interval);
        }

        _delay_ms(10);
    }
}
