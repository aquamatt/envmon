/****************
 * BMP180/BMP085 temperature/pressure module
 *
 * Usage notes:
 * - it is expected that i2c_init() will have been called by the main
 *   programme
 * - TEST can be overridden in the Makefile; set to 1 to use datasheet
 *        example values for coefficients and uncompensated readings
 */
#include <util/delay.h>
#include "i2c/i2cmaster.h"
#include "bmp180.h"

#ifndef TEST
#define TEST 0
#endif

// Chip calibration coefficients
int16_t AC1 = 0;
int16_t AC2 = 0;
int16_t AC3 = 0;
uint16_t AC4 = 0;
uint16_t AC5 = 0;
uint16_t AC6 = 0;
int16_t B1 = 0;
int16_t B2 = 0;
int16_t MB = 0;
int16_t MC = 0;
int16_t MD = 0;

/**********************
 * Convenience functions for getting typed values from an I2C unit
 */
uint8_t get_byte(uint8_t host, uint8_t address) {
    uint8_t value;
    i2c_start_wait(host+I2C_WRITE);
    i2c_write(address);
    i2c_rep_start(host+I2C_READ);
    value = i2c_readNak();
    i2c_stop();
    return value;
}


uint16_t get_word(uint8_t host, uint8_t address) {
    uint16_t value = 0;
    uint8_t part = 0;

    // get MSB
    i2c_start_wait(host+I2C_WRITE);
    i2c_write(address);
    i2c_rep_start(host+I2C_READ);
    part = i2c_readAck();
    value = part << 8;
    // get LSB
    part = i2c_readNak();
    value += part;
    i2c_stop();
    return value;
}


/********
 * Put a command into the appropriate address
 * @TODO PARAMETERISE DELAY: Fixed at 6 because compiler requires const
 * but this is only good for temp readout and low precision pressure...
 */
uint16_t execute(uint8_t host,
             uint8_t command_address,
             uint8_t command_value,
             uint8_t response_address)
{
    i2c_start_wait(host+I2C_WRITE);
    i2c_write(command_address);
    i2c_write(command_value);
    i2c_stop();
    _delay_ms(6);
    return get_word(host, response_address);
}


uint8_t _test_coefficient(uint16_t c) {
    if (c == 0xffff || c == 0x0000) {
        return 1;
    }
    return 0;
}


/************************
 * Get coefficients for the BMP085/BMP180 sensor
 */
uint8_t set_coefficients(void) {
    AC1 = get_word(I2CADDR, CAL_AC1);
    AC2 = get_word(I2CADDR, CAL_AC2);
    AC3 = get_word(I2CADDR, CAL_AC3);
    AC4 = get_word(I2CADDR, CAL_AC4);
    AC5 = get_word(I2CADDR, CAL_AC5);
    AC6 = get_word(I2CADDR, CAL_AC6);
    B1 = get_word(I2CADDR, CAL_B1);
    B2 = get_word(I2CADDR, CAL_B2);
    MB = get_word(I2CADDR, CAL_MB);
    MC = get_word(I2CADDR, CAL_MC);
    MD = get_word(I2CADDR, CAL_MD);

    // return value 0 = SUCCESS
    //              1 = ERROR
    uint8_t rv = 0;

    rv |= _test_coefficient(AC1);
    rv |= _test_coefficient(AC2);
    rv |= _test_coefficient(AC3);
    rv |= _test_coefficient(AC4);
    rv |= _test_coefficient(AC5);
    rv |= _test_coefficient(AC6);
    rv |= _test_coefficient(B1);
    rv |= _test_coefficient(B2);
    rv |= _test_coefficient(MB);
    rv |= _test_coefficient(MC);
    rv |= _test_coefficient(MD);

    return rv;
}


uint8_t set_test_coefficients(void)
{
    AC1 = 408;
    AC2 = -72;
    AC3 = -14383;
    AC4 = 32741;
    AC5 = 32757;
    AC6 = 23153;
    B1 = 6190;
    B2 = 4;
    MB = -32768;
    MC = -8711;
    MD = 2868;

    return 0;
}

/**************
 * Return the value in address CHIPTEST; should always be 0x55
 */
uint8_t test_bmp_chip(void)
{
    uint8_t result;
    result = get_byte(I2CADDR, CHIPTEST);
    if (result == 0x55)
    {
        return 0;
    }
    return result;
}


/******************
 * See:
 * https://www.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf
 * for details
 */
int32_t get_temp(void)
{
    int32_t UT = 0;
    if (TEST == 1)
    {
        UT = 27898;
    } else {
        UT = execute(I2CADDR, CONTROL, READTEMPCMD, TEMPDATA);
    }
    int32_t X1 = ((UT-(int32_t)AC6) * (int32_t)AC5) >> 15;
    int32_t X2 = ((int32_t)MC << 11) / (X1+(int32_t)MD);
    int32_t B5 = X1 + X2;
    int32_t T = (B5+8) >> 4;
    return T;
}

/**
 * Check we're got the sensor and load up the calibration coefficients
 */
uint8_t initialise_bmp_module(void) {
    uint8_t status = 0;
    status = test_bmp_chip();
    if (status == 0)
    {
        if (TEST == 1)
        {
            status = set_test_coefficients();
        } else {
            status = set_coefficients();
        }
    }

    return status;
}


