
// Define a function pointer type for the callback
typedef void (*Rotary_Encoder_callback)(int counter);

// Rotary_Encoder_statemachine
void Rotary_Encoder_statemachine_init(Rotary_Encoder_callback callback);
void Rotary_Encoder_statemachine_cleanup();