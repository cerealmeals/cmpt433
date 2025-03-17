#include "Accelerometer.h"
#include "AccelerometerInterp.h"
#include "audio_handler.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include "periodTimer.h"
#include <stdbool.h>
#include <time.h>

#define POLL_DELAY_US 100000  // 0.01s delay
#define DEBOUNCE_TIME_US 100000  // 0.01s debounce period

#define MOTION_DIFFERENCE 250

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

static uint64_t get_current_time_us() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000 + (uint64_t)ts.tv_nsec / 1000;
}

static bool check_change(int16_t current, int16_t last)
{
    if ((current > last + MOTION_DIFFERENCE) || (current < last - MOTION_DIFFERENCE)) {
        // Check if signs are different
        if ((current * last < 0) && (current != 0 && last != 0)) {
            return true;
        }
    }
    return false;
}

static void* AccelerometerInterp_threadFunc(void* arg) {
    if (arg != NULL) {
        printf("AccelerometerInterp_threadFunc should not have arguments\n");
        return NULL;
    }

    static int16_t x = 0, y = 0, z = 0;
    static uint64_t last_x_tap_time = 0, last_y_tap_time = 0, last_z_tap_time = 0;

    AccelerometerOutput output;

    while (running) {
        Accelerometer_GetTapState(&output);
        Period_markEvent(PERIOD_EVENT_ACCELEROMETER);

        uint64_t current_time = get_current_time_us();

        if (check_change(output.x, x) && (current_time - last_x_tap_time) > DEBOUNCE_TIME_US) {
            last_x_tap_time = current_time;
            audio_handler_playSound(SOUND_SNARE);
            //printf("X-axis %dtap detected!\n", x);
            // Perform action for X-axis tap
        }
        if (check_change(output.y, y) && (current_time - last_y_tap_time) > DEBOUNCE_TIME_US) {
            last_y_tap_time = current_time;
            audio_handler_playSound(SOUND_BASS_DRUM);
            //printf("Y-axis %dtap detected!\n", y);
            // Perform action for Y-axis tap
        }
        if (check_change(output.z, z) && (current_time - last_z_tap_time) > DEBOUNCE_TIME_US) {
            last_z_tap_time = current_time;
            audio_handler_playSound(SOUND_CYMBAL);
            //printf("Z-axis %dtap detected!\n", z);
            // Perform action for Z-axis tap
        }
        z = output.z;
        y = output.y;
        x = output.x;

        usleep(POLL_DELAY_US);  // Small delay to prevent excessive polling
    }

    return NULL;
}