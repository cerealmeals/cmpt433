#include <stdbool.h>
#ifndef JOYSTICK_h
#define JOYSTICK_h

// values are between -500 and 500
// pushed is true when button is pushed down
typedef struct{
    int x_value;
    int y_value;
    bool pushed;
} JoystickOutput;

void JoystickControl_GetJoystickOutput(JoystickOutput* output);

void JoystickControl_close(void);

#endif