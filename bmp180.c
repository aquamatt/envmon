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
#ifdef DEBUG
#include <stdio.h>
#endif

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

int8_t DELAY_FOR_OSS[4] = { \
    MEASURE_FAST, \
    MEASURE_STD, \
    MEASURE_SLOW, \
    MEASURE_ULTRA};

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

uint32_t get_long(uint8_t host, uint8_t address) {
    uint32_t value = 0;
    uint8_t part = 0;

    // get MSB
    i2c_start_wait(host+I2C_WRITE);
    i2c_write(address);
    i2c_rep_start(host+I2C_READ);
    part = i2c_readAck();
    value += (uint32_t)part << 16;
    // get LSB
    part = i2c_readNak();
    value += (uint32_t)part << 8;
    // get XLSB
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
void execute(uint8_t host,
             uint8_t command_address,
             uint8_t command_value,
             uint8_t measure_delay)
{
    i2c_start_wait(host+I2C_WRITE);
    i2c_write(command_address);
    i2c_write(command_value);
    i2c_stop();

    // this is only necessary because _delay_ms needs
    // to take a constant...
    switch(measure_delay) {
        case MEASURE_FAST:
            _delay_ms(MEASURE_FAST);
            break;
        case MEASURE_STD:
            _delay_ms(MEASURE_STD);
            break;
        case MEASURE_SLOW:
            _delay_ms(MEASURE_SLOW);
            break;
        default:
            _delay_ms(MEASURE_ULTRA);
    }
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

#ifdef DEBUG
    printf("AC1=%d\n", AC1);
    printf("AC2=%d\n", AC2);
    printf("AC3=%d\n", AC3);
    printf("AC4=%d\n", AC4);
    printf("AC5=%d\n", AC5);
    printf("AC6=%d\n", AC6);
    printf("B1=%d\n", B1);
    printf("B2=%d\n", B2);
    printf("MB=%d\n", MB);
    printf("MC=%d\n", MC);
    printf("MD=%d\n", MD);
#endif

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
        execute(I2CADDR, CONTROL, READTEMPCMD, MEASURE_FAST);
        UT = get_word(I2CADDR, TEMPDATA);
    }
    int32_t X1 = ((UT-(int32_t)AC6) * (int32_t)AC5) >> 15;
    int32_t X2 = ((int32_t)MC << 11) / (X1+(int32_t)MD);
    int32_t B5 = X1 + X2;
    int32_t T = (B5+8) >> 4;
    return T;
}

/**
 * OSS is one of the four operating modes defined in
 * bmp180.h
 * See:
 * https://www.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf
 * for details
 */
int32_t get_pressure(uint8_t oss) {
    int32_t UT = 0;
    int32_t UP = 0;
    int32_t p;
    if (TEST == 1)
    {
        UT = 27898;
        UP = 23843;
        oss = 0;
    } else {
        execute(I2CADDR, CONTROL, READTEMPCMD, MEASURE_FAST);
        UT = get_word(I2CADDR, TEMPDATA);
        execute(I2CADDR, CONTROL, \
                READPRESSURECMD + (oss << 6),
                DELAY_FOR_OSS[oss]);
        UP = (int32_t)get_long(I2CADDR, PRESSUREDATA);
        UP = UP >> (8 - oss);
    }
    int32_t X1 = ((UT-(int32_t)AC6) * (int32_t)AC5) >> 15;
    int32_t X2 = ((int32_t)MC << 11) / (X1+(int32_t)MD);
    int32_t B5 = X1 + X2;
    int32_t B6 = B5 - 4000;
    X1 = ((int32_t)B2 * ((B6 * B6) >> 12)) >> 11;
    X2 = (int32_t)AC2 * B6 >> 11;
    int32_t X3 = X1 + X2;
    int32_t B3 = ((((int32_t)AC1*4 + X3) << oss) + 2) >> 2 ;
    X1 = ((int32_t)AC3 * B6) >> 13;
    X2 = ((int32_t)B1 * ((B6 * B6) >> 12)) >> 16;
    X3 = ((X1 + X2) + 2) >> 2;
    uint32_t B4 = ((uint32_t)AC4 * (uint32_t)(X3+32768)) >> 15;
    uint32_t B7 = ((uint32_t)(UP - B3) * (50000 >> oss));
    if (B7 < 0x80000000)
    {
        p = (B7<<1)/B4;
    } else {
        p = (B7/B4)<<1;
    }
    X1 = (p >> 8);
    X1 *= X1;
    X1 = (X1 * 3038) >> 16;
    X2 = (-7357 * p) >> 16;
    p += (X1 + X2 + 3791) >> 4;
    return p;
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


