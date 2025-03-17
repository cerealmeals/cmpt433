#include "Accelerometer.h"
#include "AccelerometerInterp.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include "periodTimer.h"

#define POLL_DELAY_US 500000  // 0.5s delay

static void* AccelerometerInterp_threadFunc(void* arg);
static pthread_t accelerometer_thread;
static bool running = false;
static bool is_init = false;

void AccelerometerInterp_init() {
    assert(!is_init);
    running = true;
    Accelerometer_init();

    if (pthread_create(&accelerometer_thread, NULL, AccelerometerInterp_threadFunc, NULL) != 0) {
        perror("Failed to create accelerometer polling thread");
        exit(EXIT_FAILURE);
    }
    is_init = true;
}

void AccelerometerInterp_cleanup() {
    assert(is_init);
    running = false;
    
    if (pthread_join(accelerometer_thread, NULL) != 0) {
        perror("Failed to join accelerometer polling thread");
        exit(EXIT_FAILURE);
    }
    Accelerometer_cleanup();
    is_init = false;
}

static void* AccelerometerInterp_threadFunc(void* arg) {
    if (arg != NULL) {
        printf("AccelerometerInterp_threadFunc should not have arguments\n");
        return NULL;
    }

    AccelerometerOutput output;

    while (running) {
        Accelerometer_GetTapState(&output);
        Period_markEvent(PERIOD_EVENT_ACCELEROMETER);
        if (output.x_tap) {
            printf("X-axis tap detected!\n");
            // Perform action for X-axis tap
        }
        if (output.y_tap) {
            printf("Y-axis tap detected!\n");
            // Perform action for Y-axis tap
        }
        if (output.z_tap) {
            printf("Z-axis tap detected!\n");
            // Perform action for Z-axis tap
        }

        usleep(POLL_DELAY_US);  // Small delay to prevent excessive polling
    }

    return NULL;
}