#include "shutdown.h"
#include "gpio_events.h"
#include "audio_handler.h"
#include <stdio.h>

int main(){

    // initialize everything
    shutdown_init();
    
    gpio_events_init();
    audio_handler_init();

    printf("Main initialization done and is now waiting for shutdown\n");
    // wait for something to call the shutdown
    shutdown_waitForShutdown();
    
    audio_handler_cleanup();
    gpio_events_cleanup();
    shutdown_cleanup();

    printf("Main finished cleaning up\n");

    return 0;
}