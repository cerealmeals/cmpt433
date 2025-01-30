#include "JoystickInterp.h"
#include "JoystickControl.h"

Direction JoystickInterp(void)
{   
    JoystickOutput output;
    JoystickControl_GetJoystickOutput(&output);

    if(output.y_value > 400){ 
        return UP;
    }
    else if(output.y_value < -400){
        return DOWN;
    }
    else if(output.x_value > 400){
        return RIGHT;
    }
    else if(output.x_value < -400){
        return LEFT;
    }
    else{
        return NONE;
    }
}