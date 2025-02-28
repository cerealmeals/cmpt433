#include "shutdown.h"
#include "network.h"
#include "RotaryEncoder.h"
#include "Sampler.h"
#include "output.h"
#include <stdio.h>

int main(){

    // initialize everything
    shutdown_init();
    network_init(); // thread
    RotaryEncoder_init(); // thread
    Sampler_init(); // thread
    Output_init(); // thread


    
    printf("Main initialization done and is now waiting for shutdown\n");

    
    // wait for something to call the shutdown
    shutdown_waitForShutdown();

    printf("Main recevied shutdown\nCleaning up\n");

    // clean everyrthing up
    Output_cleanup();
    //printf("check-1\n");
    Sampler_cleanup();
    //printf("check0\n");
    RotaryEncoder_cleanup();
    //printf("check1\n");
    network_cleanup();
    //printf("check2\n");
    shutdown_cleanup();
    //printf("check3\n");

    printf("Main finished cleaning up\n");

    return 0;
}