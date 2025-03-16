#include "JoystickControl.h"
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

static uint16_t x_currentMax = 1600;
static uint16_t x_currentMin = 10;
static uint16_t y_currentMax = 1600;
static uint16_t y_currentMin = 10;
static int OutputMin = -500;
static int OutputMax = 500;
static int i2c_file_desc;


static bool is_init = false;

void JoystickControl_init()
{
	assert(!is_init);
	i2c_file_desc = I2C_init_bus(I2CDRV_LINUX_BUS, I2C_DEVICE_ADDRESS);
	is_init = true;
}
void JoystickControl_cleanup()
{
	assert(is_init);
	close(i2c_file_desc);
	is_init = false;
}

static int ChangeRange(uint16_t input, uint16_t max, uint16_t min)
{
	//printf("input: %d  max: %d   min: %d", input, max, min);
	int to_return = ((int)input - (int)min) * (OutputMax - OutputMin) / ((int)max - (int)min) + OutputMin;
	return to_return;
}


void JoystickControl_GetJoystickOutput(JoystickOutput* output)
{
	assert(is_init);
	// get x value
		I2C_write_reg16(i2c_file_desc, REG_CONFIGURATION, TLA2024_CHANNEL_CONF_0);
		uint16_t raw_read = I2C_read_reg16(i2c_file_desc, REG_DATA);
		// Convert byte order and shift bits into place
		uint16_t value = ((raw_read & 0xFF) << 8) | ((raw_read & 0xFF00) >> 8);
		value  = value >> 4;
		if(value > x_currentMax){
			x_currentMax = value;
		}
		else if(value < x_currentMin){
			x_currentMin = value;
		}
		(*output).x_value = ChangeRange(value, x_currentMax, x_currentMin);
	//printf("x_og: %d   x: %d\n", value, output->x_value);
	// get y value
		I2C_write_reg16(i2c_file_desc, REG_CONFIGURATION, TLA2024_CHANNEL_CONF_1);
		raw_read = I2C_read_reg16(i2c_file_desc, REG_DATA);
		// Convert byte order and shift bits into place
		value = ((raw_read & 0xFF) << 8) | ((raw_read & 0xFF00) >> 8);
		value  = value >> 4;if(value > y_currentMax){
			y_currentMax = value;
		}
		else if(value < y_currentMin){
			y_currentMin = value;
		}
		(*output).y_value = -ChangeRange(value, y_currentMax, y_currentMin);
}