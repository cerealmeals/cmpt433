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

#define CTRL1_LOCATION      0x20
#define CTRL1_WRITE         0x54

#define CTRL3_LOCATION      0x22
#define CTRL3_WRITE         0x10

#define CTRL4_INT1_PAD_CTRL_LOCATION    0x23
#define CTRL4_INT1_PAD_CTRL_WRITE       0x40

#define TAP_THS_X_LOCATION  0x30
#define TAP_THS_X_WRITE     0x00

#define TAP_THS_Y_LOCATION  0x31
#define TAP_THS_Y_WRITE     0xe0

#define TAP_THS_Z_LOCATION  0x32
#define TAP_THS_Z_WRITE     0xe0

#define INT_DUR_LOCATION    0x33
#define INT_DUR_WRTIE       0x03

#define WAKE_UP_THS_LOCATION    0x34
#define WAKE_UP_THS_WRITE       0x00

#define CTRL7_LOCATION      0x3f
#define CTRL7_WRITE         0x20

static int i2c_file_desc;
static bool is_init = false;

void Accelerometer_init()
{
    assert(!is_init);
    i2c_file_desc = I2C_init_bus(I2CDRV_LINUX_BUS, I2C_DEVICE_ADDRESS);
    I2C_write_reg8(i2c_file_desc, CTRL1_LOCATION, CTRL1_WRITE);
    I2C_write_reg8(i2c_file_desc, CTRL3_LOCATION, CTRL3_WRITE);
    I2C_write_reg8(i2c_file_desc, CTRL4_INT1_PAD_CTRL_LOCATION, CTRL4_INT1_PAD_CTRL_WRITE);
    I2C_write_reg8(i2c_file_desc, INT_DUR_LOCATION, INT_DUR_WRTIE);
    printf("check\n");
    I2C_write_reg8(i2c_file_desc, WAKE_UP_THS_LOCATION, WAKE_UP_THS_WRITE);
    I2C_write_reg8(i2c_file_desc, TAP_THS_X_LOCATION, TAP_THS_X_WRITE);
    I2C_write_reg8(i2c_file_desc, TAP_THS_Y_LOCATION, TAP_THS_Y_WRITE);
    I2C_write_reg8(i2c_file_desc, TAP_THS_Z_LOCATION, TAP_THS_Z_WRITE);
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
    //printf("tap_status (hex): 0x%02X\n", tap_status);
    // Extract tap detection bits (update bit masks as per datasheet)
    output->x_tap = (tap_status & 0x04) != 0;
    output->y_tap = (tap_status & 0x02) != 0;
    output->z_tap = (tap_status & 0x01) != 0;
}