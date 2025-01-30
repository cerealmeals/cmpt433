#include "JoystickControl.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <stdint.h>
#include <stdbool.h>
#include "I2C.h"

// Device bus & address
#define I2CDRV_LINUX_BUS "/dev/i2c-1"
#define I2C_DEVICE_ADDRESS 0x48

// Register in TLA2024
#define REG_CONFIGURATION 0x01
#define REG_DATA 0x00

// Configuration reg contents for continuously sampling different channels
#define TLA2024_CHANNEL_CONF_0 0x83C2	// Joystick Y
#define TLA2024_CHANNEL_CONF_1 0x83D2	// Joystick X
#define TLA2024_CHANNEL_CONF_2 0x83E2	// LED Receive
#define TLA2024_CHANNEL_CONF_3 0x83F2	// ADC Header pin 2



static const float multiplier = 1000.0/1635.0;
static const float constant = 809500.0/1635.0;


static bool initilized = false;
static int i2c_file_desc;

void JoystickControl_GetJoystickOutput(JoystickOutput* output)
{
	if( initilized == false){
		i2c_file_desc = I2C_init_bus(I2CDRV_LINUX_BUS, I2C_DEVICE_ADDRESS);
		initilized = true;
	}

	// get x value
		I2C_write_reg16(i2c_file_desc, REG_CONFIGURATION, TLA2024_CHANNEL_CONF_0);
		uint16_t raw_read = I2C_read_reg16(i2c_file_desc, REG_DATA);
		// Convert byte order and shift bits into place
		uint16_t value = ((raw_read & 0xFF) << 8) | ((raw_read & 0xFF00) >> 8);
		value  = value >> 4;
		(*output).x_value = (int)(((float)value*multiplier) - constant);

	// get y value
		I2C_write_reg16(i2c_file_desc, REG_CONFIGURATION, TLA2024_CHANNEL_CONF_1);
		raw_read = I2C_read_reg16(i2c_file_desc, REG_DATA);
		// Convert byte order and shift bits into place
		value = ((raw_read & 0xFF) << 8) | ((raw_read & 0xFF00) >> 8);
		value  = value >> 4;
		(*output).y_value = -(int)(((float)value*multiplier) - constant);
    
	// get push value
		// ??
}

void JoystickControl_close(void)
{
	close(i2c_file_desc);
	initilized = false;
}