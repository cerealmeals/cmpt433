#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "AccelerometerApp.h"
#include "NeopixelApp.h"
#include "shutdown.h"
#include "GameLoop.h"

int main(void)
{
    printf("=== FIND THE DOT GAME ===\n");
    shutdown_init();
    NeopixelApp_init();
    AccelerometerApp_init();
    GameLoop_init();

    printf("Waiting for shutdown...\n");
    shutdown_waitForShutdown();

    printf("Shutting down...\n");
    GameLoop_cleanup();
    AccelerometerApp_cleanup();
    NeopixelApp_cleanup();
    shutdown_cleanup();
    printf("=== Done ===\n");
    return 0;
}
