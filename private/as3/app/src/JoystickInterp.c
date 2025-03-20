#include "JoystickControl.h"
#include "audio_handler.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include "JoystickInterp.h"
#include <pthread.h>

#define POLL_DELAY_US 200000  // 0.2 delay
#define VOLUME_THRESHOLD 200 // Threshold for detecting up/down movement

static void* JoystickInterp_threadFunc(void* arg);
static pthread_t joystick_thread;
static bool running = false;
static bool is_init = false;


void JoystickInterp_init() {
    assert(!is_init);
    running = true;
    JoystickControl_init();

    if (pthread_create(&joystick_thread, NULL, JoystickInterp_threadFunc, NULL) != 0) {
        perror("Failed to create joystick polling thread");
        exit(EXIT_FAILURE);
    }
    is_init = true;
}

void JoystickInterp_cleanup() {

    assert(is_init);
    running = false;
    
    if (pthread_join(joystick_thread, NULL) != 0) {
        perror("Failed to join joystick polling thread");
        exit(EXIT_FAILURE);
    }
    JoystickControl_cleanup();
    is_init = false;
}

static void* JoystickInterp_threadFunc(void* arg)
{
    if(arg != NULL){
        printf("JoystickInterp_threadFunc should not have arguments\n");
        return NULL;
    }
    JoystickOutput output;

    while (running) {
        JoystickControl_GetJoystickOutput(&output);

        // Adjust volume based on vertical movement
        if (output.y_value > VOLUME_THRESHOLD) {
            audio_handler_adjustVolume(true);  // Increase volume
        } else if (output.y_value < -VOLUME_THRESHOLD) {
            audio_handler_adjustVolume(false); // Decrease volume
        }

        usleep(POLL_DELAY_US);  // Small delay to prevent excessive polling
    }

    return NULL;
}