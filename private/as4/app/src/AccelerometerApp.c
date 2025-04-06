#include "AccelerometerApp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>       // for usleep
#include <pthread.h>
#include <math.h>         // for abs
#include <stdbool.h>
#include <stdatomic.h>    // for _Atomic bool

#include "Accelerometer.h" // Accelerometer_init/cleanup + GetaccelerationState
#include "NeopixelApp.h"   // NeopixelApp_Init/Cleanup, SetAll, etc.

#include <assert.h>

#define NEO_NUM_LEDS    8
#define LOOP_DELAY_US   10000   // ~100 updates/second
#define THRESHOLD_IN    100
#define THRESHOLD_OUT   120
// ---------------------------------------------------------
// Globals for the background thread
// ---------------------------------------------------------

static pthread_t     Accelerometer_thread;           // The tracking thread
static bool          Accelerometer_threadShouldExit = false;

// Protect the target variables with a mutex (so SetTarget can be called safely).
static pthread_mutex_t targetMutex = PTHREAD_MUTEX_INITIALIZER;
static int16_t        targetX = 0;
static int16_t        targetY = 0;

// Use an **atomic bool** to store whether we’re on target
static atomic_bool onTarget = false;

static atomic_bool ControlNeoPixel = true;

static bool is_init = false;

// A helper function to apply hysteresis to one axis
//   - diff: The difference (e.g. x - targetX)
//   - prevWithin: Did we used to be “in”?
// Returns whether we are now “in”
static bool applyHysteresis(int diff, bool prevWithin)
{
    int magnitude = abs(diff);

    if (prevWithin) {
        // We were previously in => only exit if we exceed THRESHOLD_OUT
        if (magnitude > THRESHOLD_OUT) {
            return false; // now out
        } else {
            return true;  // stay in
        }
    } else {
        // We were previously out => only enter if we are below THRESHOLD_IN
        if (magnitude < THRESHOLD_IN) {
            return true; // now in
        } else {
            return false; // stay out
        }
    }
}

// ---------------------------------------------------------
// The background thread function
//  - runs until Accelerometer_threadShouldExit is set to true
//  - reads accelerometer
//  - compares to target
//  - updates NeoPixel
// ---------------------------------------------------------
static void* trackingThreadFunc(void* arg)
{
    (void)arg;

    uint32_t ledColors[NEO_NUM_LEDS];
    
    int N500 = -500;
    int N400 = -400;
    int N300 = -300;
    int N200 = -200; 
    int N100 = -100; 
    int P200 = 200;
    int P300 = 300;
    int P400 = 400;
    int P500 = 500;



    // This indicates whether X and Y were previously considered "within range"
    bool prevWithinX = false;
    bool prevWithinY = false;

    while (!Accelerometer_threadShouldExit) {
        // 0) Check if we want to skip controlling the NeoPixel for this iteration
        if (!ControlNeoPixel) {
            usleep(LOOP_DELAY_US);
            continue;
        }

        // 1) Read the accelerometer
        AccelerometerOutput accel;
        Accelerometer_GetAccelerationState(&accel);
        int16_t x = accel.x;
        int16_t y = accel.y;

        // 2) Read the current target
        pthread_mutex_lock(&targetMutex);
        int16_t targetXLocal = targetX;
        int16_t targetYLocal = targetY;
        pthread_mutex_unlock(&targetMutex);

        // 3) Compare with hysteresis
        int diffX = x - targetXLocal;
        int diffY = y - targetYLocal;

        bool newWithinX = applyHysteresis(diffX, prevWithinX);
        bool newWithinY = applyHysteresis(diffY, prevWithinY);

        prevWithinX = newWithinX;
        prevWithinY = newWithinY;

        // We'll build a new color pattern each loop
        memset(ledColors, 0, sizeof(ledColors));

        // If both are within => onTarget => all blue
        if (newWithinX && newWithinY) {
            onTarget = true;

            for (int i = 0; i < NEO_NUM_LEDS; i++) {
                ledColors[i] = Blue;
            }
            NeopixelApp_SetAll(ledColors);
            usleep(LOOP_DELAY_US);
            continue;
        }

        // Not on target
        onTarget = false;

        // Color logic based on Y
        uint32_t colour, lightcolour;
        if (newWithinY) {
            // If Y is "within" threshold => use Blue
            colour = Blue;
            lightcolour = LightBlue;
        } else if (diffY > 0) {
            // Y is bigger => green
            colour = Green;
            lightcolour = LightGreen;
        } else {
            // Y is smaller => red
            colour = Red;
            lightcolour = LightRed;
        }

        // If X is “within” => fill entire strip with colour
        if (newWithinX) {
            for (int i = 0; i < NEO_NUM_LEDS; i++) {
                ledColors[i] = colour;
            }
        }
        else {
            // Negative side
            if (diffX <= N500) {
                ledColors[0] = lightcolour;
                N500 = -480;
            } else if (diffX <= N400) {
                ledColors[0] = colour;
                ledColors[1] = lightcolour;
                N500 = -500;
                N400 = -380;
            } else if (diffX <= N300) {
                ledColors[0] = lightcolour;
                ledColors[1] = colour;
                ledColors[2] = lightcolour;
                N400 = -400;
                N300 = -280;
            } else if (diffX <= N200) {
                ledColors[1] = lightcolour;
                ledColors[2] = colour;
                ledColors[3] = lightcolour;
                N300 = -300;
                N200 = -180;
            } else if (diffX <= N100) {
                ledColors[2] = lightcolour;
                ledColors[3] = colour;
                ledColors[4] = lightcolour;
                N200 = -200;
                N100 = -80;
            }

            // Positive side
            else if (diffX <= P200) {
                ledColors[3] = lightcolour;
                ledColors[4] = colour;
                ledColors[5] = lightcolour;
                P200 = 220;
            } else if (diffX <= P300) {
                ledColors[4] = lightcolour;
                ledColors[5] = colour;
                ledColors[6] = lightcolour;
                P200 = 200;
                P300 = 320;
            } else if (diffX <= P400) {
                ledColors[5] = lightcolour;
                ledColors[6] = colour;
                ledColors[7] = lightcolour;
                P300 = 300;
                P400 = 420;
            } else if (diffX <= P500) {
                ledColors[6] = lightcolour;
                ledColors[7] = colour;
                P400 = 400;
                P500 = 520;
            } else{
                ledColors[7] = lightcolour;
                P500 = 500;
            }
        }
 
        // Commit
        NeopixelApp_SetAll(ledColors);
        usleep(LOOP_DELAY_US);
    }

    return NULL;
}

// ---------------------------------------------------------
// Public Functions
// ---------------------------------------------------------
void AccelerometerApp_init(void)
{
    assert(!is_init);
    // 1) Initialize accelerometer
    Accelerometer_init();

    // 2) Default target to zero
    pthread_mutex_lock(&targetMutex);
    targetX = 0;
    targetY = 0;
    pthread_mutex_unlock(&targetMutex);

    // 3) Clear onTarget
    onTarget = false;

    // 4) Start the thread
    Accelerometer_threadShouldExit = false;
    if (pthread_create(&Accelerometer_thread, NULL, trackingThreadFunc, NULL) != 0) {
        perror("ERROR: Could not create tracking thread");
        exit(EXIT_FAILURE);
    }
    is_init = true;
}

void AccelerometerApp_cleanup(void)
{
    assert(is_init);
    // Signal the thread to exit
    Accelerometer_threadShouldExit = true;
    // Wait for it
    pthread_join(Accelerometer_thread, NULL);

    // Cleanup
    Accelerometer_cleanup();
    is_init = false;
}

void AccelerometerApp_SetTarget(int16_t newTargetX, int16_t newTargetY)
{
    assert(is_init);
    pthread_mutex_lock(&targetMutex);
    targetX = newTargetX;
    targetY = newTargetY;
    pthread_mutex_unlock(&targetMutex);

    printf("AccelerometerApp_SetTarget => (%d, %d)\n", newTargetX, newTargetY);
}

bool AccelerometerApp_IsOnTarget(void)
{
    assert(is_init);
    // Just load the atomic boolean
    return onTarget;
}

void AccelerometerApp_SetControlNeoPixel(bool enable)
{
    assert(is_init);
    ControlNeoPixel = enable;
}
