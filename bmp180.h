// BMP085 default address.
#define I2CADDR            0x77 << 1

// Operating Modes
#define ULTRALOWPOWER      0
#define STANDARD           1
#define HIGHRES            2
#define ULTRAHIGHRES       3

// BMP085 Registers
#define CAL_AC1            0xAA  // R   Calibration data (16 bits)
#define CAL_AC2            0xAC  // R   Calibration data (16 bits)
#define CAL_AC3            0xAE  // R   Calibration data (16 bits)
#define CAL_AC4            0xB0  // R   Calibration data (16 bits)
#define CAL_AC5            0xB2  // R   Calibration data (16 bits)
#define CAL_AC6            0xB4  // R   Calibration data (16 bits)
#define CAL_B1             0xB6  // R   Calibration data (16 bits)
#define CAL_B2             0xB8  // R   Calibration data (16 bits)
#define CAL_MB             0xBA  // R   Calibration data (16 bits)
#define CAL_MC             0xBC  // R   Calibration data (16 bits)
#define CAL_MD             0xBE  // R   Calibration data (16 bits)
#define CONTROL            0xF4
#define TEMPDATA           0xF6
#define PRESSUREDATA       0xF6

// Delay constants
#define MEASURE_FAST    5
#define MEASURE_STD     8
#define MEASURE_SLOW    14
#define MEASURE_ULTRA   26

// Commands
#define READTEMPCMD        0x2E
#define READPRESSURECMD    0x34
#define CHIPTEST           0xD0  // value 0x55 always - used to check OK

uint8_t get_byte(uint8_t host, uint8_t address);
uint16_t get_word(uint8_t host, uint8_t address);
void execute(uint8_t host,
             uint8_t command_address,
             uint8_t command_value,
             uint8_t measure_delay);
uint8_t _test_coefficient(uint16_t c);
uint8_t set_coefficients(void);
uint8_t set_test_coefficients(void);
uint8_t initialise_bmp_module(void);
uint8_t test_bmp_chip(void);
int32_t get_temp(void);
int32_t get_pressure(uint8_t oss);
