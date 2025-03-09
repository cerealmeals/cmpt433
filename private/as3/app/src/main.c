#include "shutdown.h"
#include <stdio.h>

int main(){

    // initialize everything
    shutdown_init();
    
    


    
    printf("Main initialization done and is now waiting for shutdown\n");

    
    // wait for something to call the shutdown
    shutdown_waitForShutdown();

    printf("Main recevied shutdown\nCleaning up\n");

    
    shutdown_cleanup();

    printf("Main finished cleaning up\n");

    return 0;
}