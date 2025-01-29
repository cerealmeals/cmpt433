#include "JoystickControl.h"

// code originally from https://opencoursehub.cs.sfu.ca/bfraser/solutions/433/guide-code/i2c_adc_tla2024/tla2024_demo.c
//edited by Sam
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <stdint.h>
#include <stdbool.h>

// Device bus & address
#define I2CDRV_LINUX_BUS "/dev/i2c-1"
#define I2C_DEVICE_ADDRESS 0x48

// Register in TLA2024
#define REG_CONFIGURATION 0x01
#define REG_DATA 0x00

// Configuration reg contents for continuously sampling different channels
#define TLA2024_CHANNEL_CONF_0 0x83C2
#define TLA2024_CHANNEL_CONF_1 0x83D2
#define TLA2024_CHANNEL_CONF_2 0x83E2
#define TLA2024_CHANNEL_CONF_3 0x83F2

// Which channel to sample?
#define SELECTED_CHANNEL_CONF TLA2024_CHANNEL_CONF_0

static int init_i2c_bus(char* bus, int address);
static void write_i2c_reg16(int i2c_file_desc, uint8_t reg_addr, uint16_t value);
static uint16_t read_i2c_reg16(int i2c_file_desc, uint8_t reg_addr);


int main()
{
	printf("Read TLA2024 ADC\n");

	int i2c_file_desc = init_i2c_bus(I2CDRV_LINUX_BUS, I2C_DEVICE_ADDRESS);

	// Select the channel
	write_i2c_reg16(i2c_file_desc, REG_CONFIGURATION, SELECTED_CHANNEL_CONF);

	while(true) {

		// Read a register:
		uint16_t raw_read = read_i2c_reg16(i2c_file_desc, REG_DATA);
        printf("Raw reading: 0x%04x\n", raw_read);
	
		// Convert byte order and shift bits into place
		uint16_t value = ((raw_read & 0xFF) << 8) | ((raw_read & 0xFF00) >> 8);
		value  = value >> 4;

		printf("Raw Read: 0x%04x  -->  Reorder & shift: 0x%04x = %8d\n", raw_read, value, value);		
		// sleep(1);

	}

	// Cleanup I2C access;
	close(i2c_file_desc);
	return 0;
}

static int init_i2c_bus(char* bus, int address)
{
	int i2c_file_desc = open(bus, O_RDWR);
	if (i2c_file_desc == -1) {
		printf("I2C DRV: Unable to open bus for read/write (%s)\n", bus);
		perror("Error is:");
		exit(EXIT_FAILURE);
	}

	if (ioctl(i2c_file_desc, I2C_SLAVE, address) == -1) {
		perror("Unable to set I2C device to slave address.");
		exit(EXIT_FAILURE);
	}
	return i2c_file_desc;
}

static void write_i2c_reg16(int i2c_file_desc, uint8_t reg_addr, uint16_t value)
{
	int tx_size = 1 + sizeof(value);
	uint8_t buff[tx_size];
	buff[0] = reg_addr;
	buff[1] = (value & 0xFF);
	buff[2] = (value & 0xFF00) >> 8;
	int bytes_written = write(i2c_file_desc, buff, tx_size);
	if (bytes_written != tx_size) {
		perror("Unable to write i2c register");
		exit(EXIT_FAILURE);
	}
}

static uint16_t read_i2c_reg16(int i2c_file_desc, uint8_t reg_addr)
{
	// To read a register, must first write the address
	int bytes_written = write(i2c_file_desc, &reg_addr, sizeof(reg_addr));
	if (bytes_written != sizeof(reg_addr)) {
		perror("Unable to write i2c register.");
		exit(EXIT_FAILURE);
	}

	// Now read the value and return it
	uint16_t value = 0;
	int bytes_read = read(i2c_file_desc, &value, sizeof(value));
	if (bytes_read != sizeof(value)) {
		perror("Unable to read i2c register");
		exit(EXIT_FAILURE);
	}
	return value;
}

void JoystickControl_Output(JoystickOutput* output)
{

}