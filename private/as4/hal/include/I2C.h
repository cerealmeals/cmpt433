// This module provides functionality to initialize, communicate with, 
// and clean up an I2C bus. It supports reading from and writing to 
// 16-bit registers on I2C devices, making it suitable for interfacing 
// with hardware components like sensors or ADCs.

#include <stdint.h>
#ifndef ISC_H
#define ISC_H


// Initializes the I2C bus by opening the specified bus file and setting the slave address.
// Returns a file descriptor for the I2C bus, which is used in subsequent read/write operations.
int I2C_init_bus(char* bus, int address);

// Writes a 16-bit value to a specified register address on the I2C device.
void I2C_write_reg16(int i2c_file_desc, uint8_t reg_addr, uint16_t value);

// Writes a 8-bit value to a specified register address on the I2C device.
void I2C_write_reg8(int i2c_file_desc, uint8_t reg_addr, uint8_t value);

// Reads a 16-bit value from a specified register address on the I2C device.
uint16_t I2C_read_reg16(int i2c_file_desc, uint8_t reg_addr);

// Reads a 8-bit value from a specified register address on the I2C device.
uint16_t I2C_read_reg8(int i2c_file_desc, uint8_t reg_addr);

// Cleans up the I2C module by resetting its initialization state.
void I2C_cleanup(void);

#endif