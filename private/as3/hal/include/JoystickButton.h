
#ifndef JOYSTICK_BUTTON_H
#define JOYSTICK_BUTTON_H

// Function pointer type for button press callback.
typedef void (*JoystickButton_Callback)(int counter);

/**
 * @brief Initializes the joystick button module.
 * 
 * Sets up GPIO interrupts for the joystick button.
 * The provided callback function will be called when the button is pressed.
 * 
 * @param callback Function pointer to be called when the button is pressed.
 */
void JoystickButton_init(JoystickButton_Callback callback);

/**
 * @brief Cleans up the joystick button module.
 * 
 * Releases resources used by the joystick button, including
 * disabling button interrupts.
 */
void JoystickButton_cleanup();

#endif // JOYSTICK_BUTTON_H