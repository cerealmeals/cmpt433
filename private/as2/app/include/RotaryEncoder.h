// This module manages a rotary encoder's input and translates its changes into a volume value. 
// It also controls an LED emitter to reflect the current volume setting. 
// The module integrates with a state machine to handle encoder changes and ensures the volume stays within defined bounds.

#ifndef ROTARYENCODER_H
#define ROTARYENCODER_H

// Initializes the module, sets up GPIO, initializes the rotary encoder state machine, 
// and configures the LED emitter to reflect the initial volume.
void RotaryEncoder_init(void);

// Handles changes in the rotary encoder's position, adjusts the volume within defined bounds, 
// and updates the LED emitter to reflect the new volume.
void RotaryEncoder_handleChange(int counter);

// Returns the current volume value, allowing other modules to retrieve the current setting.
int RotartEncoder_currentVolume(void);

// Cleans up resources, shuts down the LED emitter, stops the state machine, and releases GPIO resources.
void RotaryEncoder_cleanup(void);

#endif