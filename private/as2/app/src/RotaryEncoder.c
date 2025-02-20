#include <stdio.h>
#include "gpio.h"
#include "RE_statemachine.h"

static int volume = 0;

void RotaryEncoder_init(void)
{
    // Startup & Initialization
    Gpio_initialize();
    BtnStateMachine_init();

    BtnStateMachine_doState(RotaryEncoder_handleChange);

    printf("Volume: %d\n", volume);
}

void RotaryEncoder_handleChange(int counter)
{
    printf("New volume: %d\n", counter);
}

void RotaryEncoder_clean(void)
{
    BtnStateMachine_cleanup();
    Gpio_cleanup();
}