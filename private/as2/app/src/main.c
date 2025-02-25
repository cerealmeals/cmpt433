#include "shutdown.h"
#include "network.h"
#include "RotaryEncoder.h"
#include "Sampler.h"
#include "output.h"
#include <stdio.h>

int main(){

    // initialize everything
    shutdown_init();
    network_init();
    RotaryEncoder_init();
    Sampler_init();
    Output_init();


    
    printf("Main initialization done and is now waiting for shutdown\n");

    
    // wait for something to call the shutdown
    shutdown_waitForShutdown();

    printf("Main recevied shutdown\nCleaning up\n");

    // clean everyrthing up
    Output_cleanup();
    Sampler_cleanup();
    RotaryEncoder_cleanup();
    network_cleanup();
    shutdown_cleanup();

    printf("Main finished cleaning up\n");

    return 0;
}