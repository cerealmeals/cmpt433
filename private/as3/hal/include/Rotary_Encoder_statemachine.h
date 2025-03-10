#ifndef ROTARY_ENCODER_STATEMACHINE_H
#define ROTARY_ENCODER_STATEMACHINE_H

// Define a function pointer type for the callback
typedef void (*Rotary_Encoder_callback)(int counter);

// Rotary_Encoder_statemachine
void Rotary_Encoder_statemachine_init(Rotary_Encoder_callback Roation_callback, Rotary_Encoder_callback Button_callback);
void Rotary_Encoder_statemachine_cleanup();

#endif