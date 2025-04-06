#include "NeopixelApp.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>  // for usleep
#include <stdlib.h>
#include <stdbool.h>

#include "SharedMemory.h"  // your HAL with init/cleanup & SetNeopixelColours_Blocking
#include <assert.h>


#define Slide_delay_time    30000

// A static array storing the “current” colors for all 8 LEDs.
// We'll manipulate it in each public function, then commit the changes to the R5.
static uint32_t ledColors[8] = {0};

// Timeout (ms) to wait for R5 to clear the flag
static const int timeoutMs = 2000;  // 2 seconds

static bool is_init = false;

//------------------------------------------------------------------------------
// Helper: commit the ledColors[] array to the R5, blocking until the R5 is ready
//------------------------------------------------------------------------------
static int commitColors(void)
{
    int result = SharedMemory_SetNeopixelColours_Blocking(ledColors, timeoutMs);
    if (result != 0) {
        fprintf(stderr, "ERROR: Timed out or failed to write to R5.\n");
    }
    return result;
}

//------------------------------------------------------------------------------
// Public Functions
//------------------------------------------------------------------------------
void NeopixelApp_init(void)
{
    assert(!is_init);
    // Initialize the shared memory layer
    if (SharedMemory_init() != 0) {
        fprintf(stderr, "NeopixelApp_Init: failed to initialize shared memory!\n");
        exit(EXIT_FAILURE);
    }
    // Clear the local color array and commit
    memset(ledColors, 0, sizeof(ledColors));
    commitColors();
    is_init = true;
}

void NeopixelApp_cleanup(void)
{
    assert(is_init);
    // Clear out the LEDs (optional)
    memset(ledColors, 0, sizeof(ledColors));
    commitColors();

    // Cleanup the shared memory
    SharedMemory_cleanup();
    is_init = false;
}

void NeopixelApp_SetAll(const uint32_t colors[8])
{
    assert(is_init);
    // Copy the user’s 8 colors into our local array
    memcpy(ledColors, colors, sizeof(ledColors));

    // Commit to the R5
    commitColors();
}

void NeopixelApp_SetOne(uint32_t color, int index)
{
    assert(is_init);
    if (index < 0 || index >= 8) {
        fprintf(stderr, "NeopixelApp_SetOne: invalid index %d\n", index);
        return;
    }
    ledColors[index] = color;
    commitColors();
}

// -----------------------------------------------------------------------------
// Example Animations: "chase" a single LED from index 0 to 7
// -----------------------------------------------------------------------------

void NeopixelApp_HitAnimation(void)
{
    assert(is_init);
    for (int j = 0; j < 2; j++){
        for (int i = 0; i < 8; i++) {
            memset(ledColors, 0, sizeof(ledColors));
            ledColors[i] = Green;

            if (commitColors() != 0) {
                break;
            }
            usleep(Slide_delay_time);
        }
        for (int i = 0; i < 8; i++) {
            memset(ledColors, 0, sizeof(ledColors));
            ledColors[i] = Blue;

            if (commitColors() != 0) {
                break;
            }
            usleep(Slide_delay_time);
        }
    }

    // Optionally turn them all off at the end
    memset(ledColors, 0, sizeof(ledColors));
    commitColors();
}

void NeopixelApp_MissAnimation(void)
{
    assert(is_init);
    for (int j = 0; j < 2; j++){
        for (int i = 0; i < 8; i++) {
            memset(ledColors, 0, sizeof(ledColors));
            ledColors[i] = Orange;

            if (commitColors() != 0) {
                break;
            }
            usleep(Slide_delay_time);
        }
        for (int i = 0; i < 8; i++) {
            memset(ledColors, 0, sizeof(ledColors));
            ledColors[i] = Red;

            if (commitColors() != 0) {
                break;
            }
            usleep(Slide_delay_time);
        }
    }

    // Turn off at the end
    memset(ledColors, 0, sizeof(ledColors));
    commitColors();
}
