#include <stdint.h>
#ifndef ISC_H
#define ISC_H

int I2C_init_bus(char* bus, int address);
void I2C_write_reg16(int i2c_file_desc, uint8_t reg_addr, uint16_t value);
uint16_t I2C_read_reg16(int i2c_file_desc, uint8_t reg_addr);

#endif