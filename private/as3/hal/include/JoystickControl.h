// The JoystickControl module provides an interface for reading the position of an analog joystick 
// and detecting button presses. The joystick's X and Y values are read via an I2C-based ADC (TLA2024), 
// while the button state is monitored through GPIO interrupts. This module initializes the necessary 
// hardware, provides functions to read joystick data, and manages resources upon cleanup.

#ifndef JOYSTICK_h
#define JOYSTICK_h
#include <stdbool.h>

// JoystickOutput represents the current state of the joystick, including its X and Y positions
// and whether the button is pressed. The X and Y values are scaled to a range between -500 and 500.
typedef struct {
    int x_value;   // Scaled X-axis position of the joystick
    int y_value;   // Scaled Y-axis position of the joystick
    bool pushed;   // Indicates if the joystick button is currently pressed
} JoystickOutput;

/**
 * @brief Initializes the joystick module.
 * 
 * Sets up the I2C interface for reading joystick position and configures 
 * GPIO interrupts to detect button presses. This function must be called 
 * before using any other functions in this module.
 */
void JoystickControl_init();

/**
 * @brief Releases resources used by the joystick module.
 * 
 * Closes the I2C connection and disables GPIO interrupts for the joystick button. 
 * This function should be called when the joystick module is no longer needed.
 */
void JoystickControl_cleanup();

/**
 * @brief Retrieves the current state of the joystick.
 * 
 * Reads the X and Y positions of the joystick and determines whether the button is pressed. 
 * The X and Y values are mapped to a range of -500 to 500 for consistency.
 * 
 * @param output A pointer to a `JoystickOutput` struct where the current joystick state will be stored.
 */
void JoystickControl_GetJoystickOutput(JoystickOutput* output);

#endif // JOYSTICK_h