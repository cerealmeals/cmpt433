// This module provides functionality to initialize, 
// read from, and clean up a light sensor connected via I2C. 
// It uses the ADC to measure light intensity and returns the value divided by 1000.

#ifndef LIGHTSENEOR_H
#define LIGHTSENEOR_H

// Initializes the light sensor by setting up the I2C bus and configuring the ADC.
void lightSensor_init(void);

// Reads the current light intensity from the sensor and returns the value as a normalized double.
double lightSensor_read(void);

// Cleans up resources by closing the I2C file descriptor and resetting the module's state.
void lightSensor_cleanup(void);

#endif