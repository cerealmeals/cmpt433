#include "Accelerometer.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <stdbool.h>
#include "I2C.h"
#include <assert.h>

// Device bus & address
#define I2CDRV_LINUX_BUS    "/dev/i2c-1"
#define I2C_DEVICE_ADDRESS  0x19

// Register addresses for tap detection
// #define TAP_SRC_LOCATRION   0x39

// #define CTRL1_LOCATION      0x20
// #define CTRL1_WRITE         0x54

// #define CTRL3_LOCATION      0x22
// #define CTRL3_WRITE         0x10

// #define CTRL4_INT1_PAD_CTRL_LOCATION    0x23
// #define CTRL4_INT1_PAD_CTRL_WRITE       0x40

#define CTRL6_LOCATION      0x25
#define CTRL6_WRITE         0x04

// #define TAP_THS_X_LOCATION  0x30
// #define TAP_THS_X_WRITE     0x00

// #define TAP_THS_Y_LOCATION  0x31
// #define TAP_THS_Y_WRITE     0xe0

// #define TAP_THS_Z_LOCATION  0x32
// #define TAP_THS_Z_WRITE     0xe0

// #define INT_DUR_LOCATION    0x33
// #define INT_DUR_WRTIE       0x03

// #define WAKE_UP_THS_LOCATION    0x34
// #define WAKE_UP_THS_WRITE       0x00

// #define CTRL7_LOCATION      0x3f
// #define CTRL7_WRITE         0x20

#define OUT_X_L_LOCATION    0x28
#define OUT_X_H_LOCATION    0x29
#define OUT_Y_L_LOCATION    0x2a
#define OUT_Y_H_LOCATION    0x2b
#define OUT_Z_L_LOCATION    0x2c
#define OUT_Z_H_LOCATION    0x2d

#define GRAVITY_SCALE       0.244

#define Z_AXES_SCALE        1000


static int i2c_file_desc;
static bool is_init = false;

void Accelerometer_init()
{
    assert(!is_init);
    i2c_file_desc = I2C_init_bus(I2CDRV_LINUX_BUS, I2C_DEVICE_ADDRESS);
    /* Initialization of sensor */
        I2C_write_reg8(i2c_file_desc, CTRL6_LOCATION, CTRL6_WRITE); /* CTRL6(25h): Set Full-scale to +/-2g, Low-noise
        enabled */
        /* Single-tap recognition enable */
        //I2C_write_reg8(i2c_file_desc, 0x23, 0x40); 
        /* CTRL4_INT1_PAD_CTRL(23h): Enable Single-tap interrupt */
        //I2C_write_reg8(i2c_file_desc, 0x30, 0x09); /* TAP_THS_X (30h): Threshold on X */
        //I2C_write_reg8(i2c_file_desc, 0x31, 0x09); /* TAP_THS_Y (31h): Threshold on Y and default priority*/
        //I2C_write_reg8(i2c_file_desc, 0x32, 0xe9); /* TAP_THS_Z (32h): Threshold on Z and enable all axes in tap recognition */
        //I2C_write_reg8(i2c_file_desc, 0x33, 0x02); /* INT_DUR (33h): Shock duration */
        //I2C_write_reg8(i2c_file_desc, 0x3f, 0x20); /* CTRL7 (3Fh): Enable interrupts */

        /* Start sensor */
        I2C_write_reg8(i2c_file_desc, 0x20, 0x74); /* CTRL1(20h): Set ODR 400Hz, High performance mode
        (14 bit) */
        
        
        
    is_init = true;
}

void Accelerometer_cleanup()
{
    assert(is_init);
    close(i2c_file_desc);
    is_init = false;
}
static int16_t formatted_output(uint8_t high_register, uint8_t low_register)
{
    int16_t upper = (int16_t)(I2C_read_reg16(i2c_file_desc, high_register) << 8);
    int16_t lower = (int16_t)(I2C_read_reg16(i2c_file_desc, low_register));
    int16_t shifted = ((upper | lower) >> 2);
    return shifted * GRAVITY_SCALE;
}
void Accelerometer_GetTapState(AccelerometerOutput* output)
{
    assert(is_init);
    int16_t minus_z = Z_AXES_SCALE;
    output->x = formatted_output(OUT_X_H_LOCATION, OUT_X_L_LOCATION);
    output->y = formatted_output(OUT_Y_H_LOCATION, OUT_Y_L_LOCATION);
    output->z = formatted_output(OUT_Z_H_LOCATION, OUT_Z_L_LOCATION) - minus_z;
}