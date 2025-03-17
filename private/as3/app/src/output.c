#include "output.h"
#include "periodTimer.h"
#include "audio_handler.h"
#include "LCD.h"
#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <stdbool.h>

#define MICRO_TO_SECOND 1000

typedef char* (*ScreenFunction)();  // Function pointer type for screens

// Buffers for formatted strings
static char screen_buffer[128];

static pthread_mutex_t output_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t output_thread;
static bool is_init = false;
static volatile sig_atomic_t shutdown_requested = 0;

static int beat_mode = 0;
static int volume = 0;
static int bpm = 0;

static double audio_min = 0, audio_max = 0, audio_avg = 0;
static int audio_samples = 0;

static double accel_min = 0, accel_max = 0, accel_avg = 0;
static int accel_samples = 0;

typedef enum{
    STATUS = 0,
    AUDIO,
    ACCELEROMETER,
    NUMBER_OFLCD_MODES
}LCDmode;

static LCDmode LCDmode_index = STATUS;

const char* current_beat[] = {
    "None",
    "Rock",
    "Custom",
};


static void * thread_function(void* arg);

// **Screen 1: Status Screen**
char* get_status_screen() {
    snprintf(screen_buffer, sizeof(screen_buffer), 
             "Beat: %s\n \n \n \n \n%d        %d", current_beat[beat_mode], volume, bpm);
    return screen_buffer;
}

// **Screen 2: Audio Timing**
char* get_audio_timing_screen() {
    snprintf(screen_buffer, sizeof(screen_buffer), 
             "Audio Timing\n \nMin: %.3f\nMax: %.3f\nAvg: %.3f\nSample: %d", 
             audio_min, audio_max, audio_avg, audio_samples);
    return screen_buffer;
}

// **Screen 3: Accelerometer Timing**
char* get_accel_timing_screen() {
    snprintf(screen_buffer, sizeof(screen_buffer), 
             "Accel. Timing\n \nMin: %.3f\nMax: %.3f\nAvg: %.3f\nSample: %d", 
             accel_min, accel_max, accel_avg, accel_samples);
    return screen_buffer;
}

// Array of function pointers
ScreenFunction screens[] = {
    get_status_screen,
    get_audio_timing_screen,
    get_accel_timing_screen
};

void Output_init(void)
{
    assert(!is_init);
    LCD_init();
    if (pthread_create(&output_thread, NULL, thread_function, NULL) != 0){
        perror("Failed to create thread in network_listener\n");
        exit(EXIT_FAILURE);
    }

    is_init = true;
}

void Output_cleanup(void)
{
    assert(is_init);
    LCD_cleanup();
    shutdown_requested = 1;
    //printf("output shutdown requested\n");
    
    if (pthread_join(output_thread, NULL) != 0) {
        perror("pthread_join failed");
        exit(EXIT_FAILURE);
    }
    
    is_init = false;
}

void Output_cycle_LCD_mode()
{
    pthread_mutex_lock(&output_mutex);
    LCDmode_index = (LCDmode_index + 1) % NUMBER_OF_MODES;
    pthread_mutex_unlock(&output_mutex);
}


static void MyMicroSleep(double mSeconds)
{
    struct timespec reqDelay;
    reqDelay.tv_sec = (time_t)(mSeconds / 1000.0);  // Convert ms to sec
    reqDelay.tv_nsec = (long)((mSeconds - (reqDelay.tv_sec * 1000.0)) * 1e6);
    nanosleep(&reqDelay, (struct timespec *) NULL);
}

static void Set_data()
{
    beat_mode = audio_handler_getBeatMode();
    volume = audio_handler_getVolume();
    bpm = audio_handler_getBPM();

    Period_statistics_t Audio_stats;
    Period_getStatisticsAndClear(PERIOD_EVENT_AUDIO, &Audio_stats);
    audio_min =  Audio_stats.minPeriodInMs;
    audio_max = Audio_stats.maxPeriodInMs;
    audio_avg = Audio_stats.avgPeriodInMs;
    audio_samples = Audio_stats.numSamples;

    Period_statistics_t Accelerometer_stats;
    Period_getStatisticsAndClear(PERIOD_EVENT_ACCELEROMETER, &Accelerometer_stats);
    accel_min = Accelerometer_stats.minPeriodInMs;
    accel_max = Accelerometer_stats.maxPeriodInMs;
    accel_avg = Accelerometer_stats.avgPeriodInMs;
    accel_samples = Accelerometer_stats.numSamples;
}

static void * thread_function(void* arg)
{
    if(arg != NULL){
        perror("Arg should be something\n");
        return NULL;
    }

    while(!shutdown_requested){

        Set_data();

        LCD_updateScreen(screens[LCDmode_index]());
        printf("M%d %dbpm vol:%d Audio[%.3f, %.3f] avg %.3f/%d Accel[%.3f, %.3f] avg %.3f/%d\n",
            beat_mode, bpm, volume, 
            audio_min, audio_max, audio_avg, audio_samples,
            accel_min, accel_max, accel_avg, accel_samples);
        
        MyMicroSleep(MICRO_TO_SECOND);
    }
    return NULL;
}