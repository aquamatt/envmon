/******
 * Build notes
 * - F_CPU should probably be overridden in Makefile
 */
#ifndef F_CPU
#define F_CPU 1000000UL
#endif

#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>

#include "i2c/i2cmaster.h"
#include "mydisplay.h"
#include "bmp180.h"
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

int main (void)
{
    // output LED configuration
    DDRB = 0b00011111;
    i2c_init();
    uart_init();
    redirect_stdout();
    verify_bmp();

    uint32_t temperature;
    uint32_t pressure;
    uint32_t last_temperature = 0;
    uint32_t last_pressure = 0;

    /*****
     * @TODO: Timing loop for the flash detector
     */
    while(1){
        temperature = get_temp();
        //pressure = get_pressure(ULTRALOWPOWER);
        pressure = get_pressure(ULTRAHIGHRES);

        if (temperature != last_temperature)
        {
            printf("T=%ld\n", temperature);
            display((uint8_t)(((float)temperature/10.0)+0.5));
            last_temperature = temperature;
        }
        if (pressure != last_pressure)
        {
            printf("P=%ld\n", pressure);
            last_pressure = pressure;
        }
        _delay_ms(1000);
    }
}
