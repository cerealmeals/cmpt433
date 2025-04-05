#ifndef ACCELEROMETER_APP_H
#define ACCELEROMETER_APP_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Initializes the accelerometer and NeoPixel systems, and starts a
 *        background thread that continuously reads the accelerometer and
 *        updates the NeoPixel strip.
 * 
 * Must be called before using other AccelerometerApp_* functions.
 */
void AccelerometerApp_init(void);

/**
 * @brief Cleans up both accelerometer and NeoPixel resources, and stops the
 *        background thread.
 */
void AccelerometerApp_cleanup(void);

/**
 * @brief Sets the target X and Y values. The background thread will use these
 *        values to decide if we are “on target” or not, and update the
 *        NeoPixel color accordingly.
 */
void AccelerometerApp_SetTarget(int16_t newTargetX, int16_t newTargetY);

/**
 * @brief Returns true if the accelerometer reading is within ±100 of the target
 *        along both X and Y (i.e. “on target”).
 */
bool AccelerometerApp_IsOnTarget(void);

/**
 * @brief Sets whether the accelerometer logic should control the NeoPixel strip.
 * 
 * @param enable If true, the loop will continue controlling the NeoPixel.
 *               If false, the loop can skip controlling the NeoPixel (allowing 
 *               other modules to override or show something else).
 */
void AccelerometerApp_SetControlNeoPixel(bool enable);

#endif // ACCELEROMETER_APP_H
