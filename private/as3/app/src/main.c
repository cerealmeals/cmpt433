#include "shutdown.h"
#include "gpio_events.h"
#include "audio_handler.h"
#include <stdio.h>
#include "JoystickInterp.h"
#include "AccelerometerInterp.h" 
#include "output.h"
#include "periodTimer.h"
#include "network.h"

int main(){

    // initialize everything
    shutdown_init();
    Period_init();
    gpio_events_init();
    audio_handler_init();
    JoystickInterp_init();
    AccelerometerInterp_init();
    Output_init();
    network_init();

    printf("Main initialization done and is now waiting for shutdown\n");
    // wait for something to call the shutdown
    shutdown_waitForShutdown();
    
    network_cleanup();
    Output_cleanup();
    AccelerometerInterp_cleanup();
    JoystickInterp_cleanup();
    audio_handler_cleanup();
    gpio_events_cleanup();
    Period_cleanup();
    shutdown_cleanup();

    printf("Main finished cleaning up\n");

    return 0;
}