// Interperate the Joystick results to a Direction

typedef enum{
    NONE = 0,
    UP,
    DOWN,
    RIGHT,
    LEFT
}Direction;

Direction JoystickInterp(void);