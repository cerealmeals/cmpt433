// This module implements a state machine to track the position of a rotary encoder. 
// It uses GPIO interrupts to detect changes in the encoder's state and updates a counter accordingly. 
// The module also supports a callback mechanism to notify other parts of the system about changes in the encoder's position.

#ifndef RE_STATEMACHINE_H_
#define RE_STATEMACHINE_H_

#include <stdbool.h>

// Define a function pointer type for the callback
typedef void (*StateMachineCallback)(int counter);


// Initializes the state machine by setting up GPIO pins for the rotary encoder and preparing the state machine for operation.
void RE_StateMachine_init(void);

// Cleans up resources by stopping the state machine thread, closing GPIO pins, and resetting the module's state.
void RE_StateMachine_cleanup(void);

// Returns the current value of the rotary encoder counter.
int RE_StateMachine_getValue(void);

// Starts the state machine on a background thread and invokes the provided callback whenever the encoder's position changes.
int RE_StateMachine_doState(StateMachineCallback callback);

#endif