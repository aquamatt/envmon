/******
 * Build notes
 * - F_CPU must be defined in the Makefile
 */
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

    uint16_t temperature;
    uint16_t last_temperature = 0;

    /*****
     * @TODO: Pressure calculation
     * @TODO: Timing loop for the flash detector
     */
    while(1){
        temperature = get_temp();

        if (temperature != last_temperature)
        {
            flash((uint8_t)(((float)temperature/10.0)+0.5), 2);
            printf("%d\n", temperature);
            last_temperature = temperature;
        }
        _delay_ms(1000);
    }
}
