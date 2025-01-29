#include <stdbool.h>
#ifndef JOYSTICK.h
#define JOYSTICK.h

typedef struct{
    int x_value;
    int y_value;
    bool push;
} JoystickOutput;

void JoystickControl_GetJoystickOutput(JoystickOutput* output);

#endif