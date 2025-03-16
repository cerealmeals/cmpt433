// The AccelerometerInterp module provides an application-level interface for handling tap events
// detected by an I2C-based accelerometer. It initializes the accelerometer, starts a background
// polling thread, and processes detected tap events on the X, Y, and Z axes.

#ifndef ACCELEROMETER_INTERP_H
#define ACCELEROMETER_INTERP_H

/**
 * @brief Initializes the accelerometer interpretation module.
 * 
 * This function initializes the accelerometer and starts a polling thread
 * that continuously checks for tap events. It must be called before using
 * any other functions in this module.
 */
void AccelerometerInterp_init();

/**
 * @brief Cleans up the accelerometer interpretation module.
 * 
 * This function stops the polling thread and releases resources used by
 * the accelerometer. It should be called when the module is no longer needed.
 */
void AccelerometerInterp_cleanup();

#endif // ACCELEROMETER_INTERP_H