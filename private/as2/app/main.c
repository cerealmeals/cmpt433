#include "shutdown.h"
#include "network.h"

int main(){

    // initialize everything
    shutdown_init();

    // wait for something to call the shutdown
    shutdown_waitForShutdown();

    // clean everyrthing up
    shutdown_clean();

    return 0;
}