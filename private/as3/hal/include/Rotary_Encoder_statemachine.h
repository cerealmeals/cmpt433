#ifndef ROTARY_ENCODER_STATEMACHINE_H
#define ROTARY_ENCODER_STATEMACHINE_H

// Define a function pointer type for the callback
typedef void (*Rotary_Encoder_callback)(int counter);

// Rotary Encoder State Machine
void Rotary_Encoder_statemachine_init(Rotary_Encoder_callback Rotation_callback);
void Rotary_Encoder_statemachine_cleanup();

#endif