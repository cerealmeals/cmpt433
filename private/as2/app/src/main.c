#include "shutdown.h"
#include "network.h"
#include <stdio.h>

int main(){

    // initialize everything
    shutdown_init();
    network_init();

    network_listener();
    printf("Main initialization done and is now waiting for shutdown\n");
    
    // wait for something to call the shutdown
    shutdown_waitForShutdown();

    printf("Main recevied shutdown\nCleaning up\n");
    // clean everyrthing up
    network_clean();
    shutdown_clean();

    printf("Main finished cleaning up\n");

    return 0;
}