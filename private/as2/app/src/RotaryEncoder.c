#include <stdio.h>
#include "gpio.h"
#include "RE_statemachine.h"
#include "RotaryEncoder.h"
#include "LED_Emitter.h"

static int last_count = 0;
static int volume = 10;
static int max = 50;
static int min = 0;

void RotaryEncoder_init(void)
{
    // Startup & Initialization
    Gpio_initialize();
    RE_StateMachine_init();
    LED_Emitter_init();
    LED_Emitter_setTimesPerSecond(volume, GPIO12);

    RE_StateMachine_doState(RotaryEncoder_handleChange);

    printf("Volume: %d\n", volume);
}

void RotaryEncoder_handleChange(int counter)
{
    if(last_count > counter && volume < max){
        volume++;
        printf("New volume: %d \n", volume);
        LED_Emitter_setTimesPerSecond(volume, GPIO12);
    }
    else if(last_count < counter && volume > min){
        volume--;
        printf("New volume: %d \n", volume);
        LED_Emitter_setTimesPerSecond(volume, GPIO12);
    }
    last_count = counter;
}

void RotaryEncoder_cleanup(void)
{
    LED_Emitter_cleanup();
    RE_StateMachine_cleanup();
    Gpio_cleanup();
}