#include "shutdown.h"
#include <stdio.h>
#include "Watch_gpio.h"
#include "Rotary_Encoder_statemachine.h"

static int last_count = 0;
static int volume = 10;
static int max = 50;
static int min = 0;

void button()
{
    printf("button has been clicked\n");
    shutdown_shutdown();
}

void Rotation_handleChange(int counter)
{
    if(last_count > counter && volume < max){
        volume++;
        printf("New volume: %d \n", volume);
    }
    else if(last_count < counter && volume > min){
        volume--;
        printf("New volume: %d \n", volume);
    }
    last_count = counter;
}


int main(){

    // initialize everything
    shutdown_init();
    
    Rotary_Encoder_statemachine_init(Rotation_handleChange, button);
    Watch_gpio_Start_Watching();
    
    printf("Main initialization done and is now waiting for shutdown\n");
    // wait for something to call the shutdown
    shutdown_waitForShutdown();

    printf("Main recevied shutdown\nCleaning up\n");

    Watch_gpio_cleanup();
    shutdown_cleanup();

    printf("Main finished cleaning up\n");

    return 0;
}