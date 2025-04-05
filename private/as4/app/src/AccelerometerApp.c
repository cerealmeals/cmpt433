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

#define NEO_NUM_LEDS    8
#define LOOP_DELAY_US   10000   // ~100 updates/second
#define THRESHOLD       100

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

// ---------------------------------------------------------
// Helper: clamp LED index to [0..(NEO_NUM_LEDS-1)]
// ---------------------------------------------------------
static int clampLedIndex(int idx)
{
    if (idx < 0) return 0;
    if (idx >= NEO_NUM_LEDS) return NEO_NUM_LEDS - 1;
    return idx;
}

// ---------------------------------------------------------
// Helper: map difference from 0..1000 => 0..(NEO_NUM_LEDS-1)
// ---------------------------------------------------------
static int mapDifferenceToLedIndex(int diff)
{
    if (diff < 0) diff = -diff;  // absolute
    if (diff > 1000) diff = 1000; 

    // scale 0..1000 => 0..7
    int scaled = (diff * (NEO_NUM_LEDS - 1)) / 1000; 
    return clampLedIndex(scaled);
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

    while (!Accelerometer_threadShouldExit) {

         // 0) Check if we want to skip controlling the NeoPixel for this iteration
        if (!ControlNeoPixel) {
            // If set, skip the rest of the loop body
            continue;
        }

        // 1) Read the accelerometer
        AccelerometerOutput accel;
        Accelerometer_GetaccelerationState(&accel);
        int16_t x = accel.x;
        int16_t y = accel.y;

        // 2) Read the current target
        pthread_mutex_lock(&targetMutex);
        int16_t targetXLocal = targetX;
        int16_t targetYLocal = targetY;
        pthread_mutex_unlock(&targetMutex);

        // 3) Compare
        int diffX = x - targetXLocal;
        int diffY = y - targetYLocal;
        bool withinX = (abs(diffX) < THRESHOLD);
        bool withinY = (abs(diffY) < THRESHOLD);

        // We'll build a new color pattern each loop
        memset(ledColors, 0, sizeof(ledColors));

        // On target => all blue
        if (withinX && withinY) {
            
            onTarget = true;
            for (int i = 0; i < NEO_NUM_LEDS; i++) {
                ledColors[i] = Blue;
            }
            NeopixelApp_SetAll(ledColors);
            usleep(LOOP_DELAY_US);
            continue;
        }
        onTarget = false;
        // Not on target get colour and which lights to turn on
        uint32_t colour = 0;
        uint32_t lightcolour = 0;

        if(withinY){
            colour = Blue;
            lightcolour = LightBlue;
        }
        else if (diffY > 0){
            colour = Red;
            lightcolour = LightRed;
        }
        else{
            colour = Green;
            lightcolour = LightGreen;
        }

        if(withinX){ // meaning diffx is [-99...99]
            for (int i = 0; i < NEO_NUM_LEDS; i++) {
                ledColors[i] = colour;
            }
        }

        // 2) Negative side
        else if (diffX <= -500) {
            ledColors[7] = lightcolour;
        } else if (diffX <= -400) {
            ledColors[7] = colour;
            ledColors[6] = lightcolour;
        } else if (diffX <= -300) {
            ledColors[7] = lightcolour;
            ledColors[6] = colour;
            ledColors[5] = lightcolour;
        } else if (diffX <= -200) {
            ledColors[6] = lightcolour;
            ledColors[5] = colour;
            ledColors[4] = lightcolour;
        } else if (diffX <= -100) {
            ledColors[5] = lightcolour;
            ledColors[4] = colour;
            ledColors[3] = lightcolour;
        }

        // 3) Positive side
        else if (diffX < 200) {          // i.e. 100..199
            ledColors[4] = lightcolour;
            ledColors[3] = colour;
            ledColors[2] = lightcolour;
        } else if (diffX < 300) {   // 200..299
            ledColors[3] = lightcolour;
            ledColors[2] = colour;
            ledColors[1] = lightcolour;
        } else if (diffX < 400) {   // 300..399
            ledColors[2] = lightcolour;
            ledColors[1] = colour;
            ledColors[0] = lightcolour;
        } else if (diffX < 500) {   // 400..499
            ledColors[1] = lightcolour;
            ledColors[0] = colour;
        } else if (diffX < 600) {   // 500..599
            ledColors[0] = lightcolour;
        }
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
    // 1) Initialize accelerometer
    Accelerometer_init();

    // 2) Initialize NeoPixel
    NeopixelApp_Init();

    // 3) Default target to zero
    pthread_mutex_lock(&targetMutex);
    targetX = 0;
    targetY = 0;
    pthread_mutex_unlock(&targetMutex);

    // 4) Clear onTarget
    atomic_store_explicit(&onTarget, false, memory_order_relaxed);

    // Optionally turn off all LEDs
    uint32_t off[NEO_NUM_LEDS];
    memset(off, 0, sizeof(off));
    NeopixelApp_SetAll(off);

    // 5) Start the thread
    Accelerometer_threadShouldExit = false;
    if (pthread_create(&Accelerometer_thread, NULL, trackingThreadFunc, NULL) != 0) {
        perror("ERROR: Could not create tracking thread");
        exit(EXIT_FAILURE);
    }
}

void AccelerometerApp_cleanup(void)
{
    // Signal the thread to exit
    Accelerometer_threadShouldExit = true;
    // Wait for it
    pthread_join(Accelerometer_thread, NULL);

    // Turn off all LEDs
    uint32_t off[NEO_NUM_LEDS];
    memset(off, 0, sizeof(off));
    NeopixelApp_SetAll(off);

    // Cleanup
    Accelerometer_cleanup();
    NeopixelApp_Cleanup();
}

void AccelerometerApp_SetTarget(int16_t newTargetX, int16_t newTargetY)
{
    pthread_mutex_lock(&targetMutex);
    targetX = newTargetX;
    targetY = newTargetY;
    pthread_mutex_unlock(&targetMutex);

    printf("AccelerometerApp_SetTarget => (%d, %d)\n", newTargetX, newTargetY);
}

bool AccelerometerApp_IsOnTarget(void)
{
    // Just load the atomic boolean
    return onTarget;
}

void AccelerometerApp_SetControlNeoPixel(bool enable)
{
    ControlNeoPixel = enable;
}
