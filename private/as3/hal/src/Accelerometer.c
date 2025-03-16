#include "Accelerometer.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <stdint.h>
#include "I2C.h"
#include <assert.h>

// Device bus & address
#define I2CDRV_LINUX_BUS    "/dev/i2c-1"
#define I2C_DEVICE_ADDRESS  0x19

// Register addresses for tap detection
#define TAP_SRC_LOCATRION   0x39

#define TAP_THS_X_LOCATION  0x30
#define TAP_THS_X_write     0x6f

#define TAP_THS_Y_LOCATION  0x31
#define TAP_THS_Y_write     0xef

#define TAP_THS_Z_LOCATION  0x32
#define TAP_THS_Z_write     0xef

static int i2c_file_desc;
static bool is_init = false;

void Accelerometer_init()
{
    assert(!is_init);
    i2c_file_desc = I2C_init_bus(I2CDRV_LINUX_BUS, I2C_DEVICE_ADDRESS);
    I2C_write_reg8(i2c_file_desc, TAP_THS_X_LOCATION, TAP_THS_X_write);
    I2C_write_reg8(i2c_file_desc, TAP_THS_Y_LOCATION, TAP_THS_Y_write);
    I2C_write_reg8(i2c_file_desc, TAP_THS_Z_LOCATION, TAP_THS_Z_write);
    is_init = true;
}

void Accelerometer_cleanup()
{
    assert(is_init);
    close(i2c_file_desc);
    is_init = false;
}

void Accelerometer_GetTapState(AccelerometerOutput* output)
{
    assert(is_init);

    // Read tap status register
    uint8_t tap_status = I2C_read_reg8(i2c_file_desc, TAP_SRC_LOCATRION);
    printf("tap_status (hex): 0x%02X\n", tap_status);
    // Extract tap detection bits (update bit masks as per datasheet)
    output->x_tap = (tap_status & 0x04) != 0;
    output->y_tap = (tap_status & 0x02) != 0;
    output->z_tap = (tap_status & 0x01) != 0;
}