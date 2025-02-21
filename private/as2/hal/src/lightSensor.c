#include "lightSensor.h"
#include "I2C.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <stdint.h>

#include <unistd.h>






// Device bus & address
#define I2CDRV_LINUX_BUS "/dev/i2c-1"
#define I2C_DEVICE_ADDRESS 0x48

// Configuration reg contents for continuously sampling different channels
#define TLA2024_CHANNEL_CONF_2 0x83E2	// LED Receive

// Register in TLA2024
#define REG_CONFIGURATION 0x01
#define REG_DATA 0x00

static bool is_init = false;
static int i2c_file_desc;

void lightSensor_init(void)
{
    assert(!is_init);
    i2c_file_desc = I2C_init_bus(I2CDRV_LINUX_BUS, I2C_DEVICE_ADDRESS);

    is_init = true;
}

int lightSensor_read(void)
{
    I2C_write_reg16(i2c_file_desc, REG_CONFIGURATION, TLA2024_CHANNEL_CONF_2);
    uint16_t raw_read = I2C_read_reg16(i2c_file_desc, REG_DATA);
    // Convert byte order and shift bits into place
    uint16_t value = ((raw_read & 0xFF) << 8) | ((raw_read & 0xFF00) >> 8);
    value  = value >> 4;
    return (int)value;
    assert(is_init);
}

void lightSensor_cleanup(void)
{
    assert(is_init);
    close(i2c_file_desc);
    is_init = false;
}