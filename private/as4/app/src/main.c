#include <stdio.h>
#include "NeopixelApp.h"
#include <unistd.h>  // for usleep

int main(void)
{
    printf("=== NeoPixelApp Demo ===\n");

    NeopixelApp_Init();

    // // 1) Set all to red (example: 0x000F0000)
    // uint32_t allRed[8];
    // for (int i = 0; i < 8; i++) {
    //     allRed[i] = Red;
    // }
    // NeopixelApp_SetAll(allRed);

    // // 2) Wait 1 second
    // sleep(1);

    // // 3) Single LED to green at position 4
    // NeopixelApp_SetOne(Green, 4); // green in your color ordering

    // // 4) Wait 1 second
    // usleep(1000000);

    // 5) Animate blue chase
    //NeopixelApp_HitAnimation();

    usleep(1000000);

    // 6) Animate red chase
    NeopixelApp_MissAnimation();

    NeopixelApp_Cleanup();
    return 0;
}
