// The Accelerometer module provides an interface for detecting tap events along the X, Y, and Z axes using an I2C-based accelerometer.
// The module includes functions to initialize and clean up resources, read accelerometer tap events, and configure sensitivity settings.

#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

#include <stdint.h>

// AccelerometerOutput stores the tap detection states along the X, Y, and Z axes.
typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} AccelerometerOutput;

/**
 * @brief Initializes the accelerometer module.
 * 
 * This function sets up the I2C connection for reading accelerometer data
 * and configures tap detection settings. The module must be initialized
 * before reading tap events.
 */
void Accelerometer_init();

/**
 * @brief Cleans up the accelerometer module.
 * 
 * This function releases resources used by the accelerometer module, including
 * closing the I2C connection.
 */
void Accelerometer_cleanup();

/**
 * @brief Reads the current tap detection state.
 * 
 * This function retrieves the latest tap detection status along the X, Y, and Z axes.
 * 
 * @param output A pointer to an `AccelerometerOutput` struct where the results will be stored.
 */
void Accelerometer_GetTapState(AccelerometerOutput* output);

#endif
