#ifndef JOYSTICK_INTERP_H
#define JOYSTICK_INTERP_H

/**
 * @brief Initializes the joystick audio control module.
 * 
 * This function starts a background thread that continuously polls the joystick
 * to adjust volume when pressed up or down and detects button presses to cycle
 * through drum beat modes.
 */
void JoystickInterp_init();

/**
 * @brief Cleans up the joystick audio control module.
 * 
 * This function stops the background polling thread and releases any resources used.
 */
void JoystickInterp_cleanup();

#endif // JOYSTICK_AUDIO_CONTROL_H