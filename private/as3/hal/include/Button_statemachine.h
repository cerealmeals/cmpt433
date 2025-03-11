#ifndef BUTTON_STATEMACHINE_H
#define BUTTON_STATEMACHINE_H

// Define a function pointer type for the callback
typedef void (*Button_callback)(int counter);

// Button State Machine
void Button_statemachine_init(Button_callback Button_callback);
void Button_statemachine_cleanup();

#endif
