#include "Sampler.h"
#include "lightSensor.h"
#include "pthread.h"
#include "periodTimer.h"
#include "RotaryEncoder.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdatomic.h>
#include <signal.h>
#include <time.h>
#include <stdbool.h>


static double average;
static bool average_init = false;
static const double smoothing = 0.999;
static long long totalCount = 0;
static int current_size = 0;
static int history_size = 0;
static int buffer_size = 500;
static double * history;
static double * current_data;

static int current_dips = 0;
static int history_dips = 0;
static bool currently_dipped = false;
static double last_sample = 0;
static const double hysteresis = 0.03;
static const double check_for_dip = 0.1;

static pthread_t sampler_thread;
static volatile sig_atomic_t shutdown_requested = 0;
static pthread_mutex_t history_mutex;
static bool is_init = false;

static void * thread_function(void* arg);

void Sampler_init(void)
{
    assert(!is_init);
    lightSensor_init();
    Period_init();
    pthread_mutex_init(&history_mutex, NULL);
    
    history = (double *)malloc(buffer_size * sizeof(double));

    if (history == NULL) {
        // Handle memory allocation failure
        fprintf(stderr, "Memory allocation failed!\n");
        exit(EXIT_FAILURE);
    }

    current_data = (double *)malloc(buffer_size * sizeof(double));

    if (current_data == NULL) {
        // Handle memory allocation failure
        fprintf(stderr, "Memory allocation failed!\n");
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&sampler_thread, NULL, thread_function, NULL) != 0) {
        perror("Failed to create thread\n");
        exit(EXIT_FAILURE);
    }

    is_init = true;
}

void Sampler_cleanup(void)
{
    assert(is_init);

    shutdown_requested = 1;
    //printf("Sampler shutdown requested\n");
    
    if (pthread_join(sampler_thread, NULL) != 0) {
        perror("pthread_join failed");
        exit(EXIT_FAILURE);
    }
    //printf("Sampler joined\n");

    pthread_mutex_lock(&history_mutex);
    if (history != NULL) {
        free(history);
        history = NULL; // Avoid dangling pointer
    }
    pthread_mutex_unlock(&history_mutex);

    if (current_data != NULL) {
        free(current_data);
        current_data = NULL; // Avoid dangling pointer
    }
    pthread_mutex_destroy(&history_mutex);
    Period_cleanup();
    lightSensor_cleanup();
    is_init = false;
}

void Sampler_moveCurrentDataToHistory(void)
{
    assert(is_init);
    assert(current_size <= buffer_size);

    pthread_mutex_lock(&history_mutex);
    for(int i = 0; i < current_size; i++){
        history[i] = current_data[i];
    }
    pthread_mutex_unlock(&history_mutex);
    history_size = current_size;
    current_size = 0;
    history_dips = current_dips;
    current_dips = 0;
}

int Sampler_getHistorySize(void)
{
    assert(is_init);

    return history_size;
}

double* Sampler_getHistory(int *size)
{
    assert(is_init);

    int return_size = *size;

    double * ret = (double*)malloc(return_size * sizeof(double));

    if(return_size > history_size){
        return_size = history_size; 
    }

    pthread_mutex_lock(&history_mutex);
    for(int i = 0; i < return_size; i++){
        ret[i] = history[i];
    }
    pthread_mutex_unlock(&history_mutex);
    return ret;
}

double Sampler_getAverageReading(void)
{
    assert(is_init);

    return average;
}


long long Sampler_getNumSamplesTaken(void)
{
    assert(is_init);

    return totalCount;
}

int Sampler_getdips(void)
{
    return history_dips;
}

static void MyNanoSleep(double mSeconds)
{
    struct timespec reqDelay;
    reqDelay.tv_sec = (time_t)(mSeconds / 1000.0);  // Convert ms to sec
    reqDelay.tv_nsec = (long)((mSeconds - (reqDelay.tv_sec * 1000.0)) * 1e6);
    nanosleep(&reqDelay, (struct timespec *) NULL);
}

static void * thread_function(void* arg)
{
    if(arg != NULL){
        perror("Arg should be something\n");
        return NULL;
    }

    struct timespec last_time, current_time;
    double sample;
    clock_gettime(CLOCK_MONOTONIC, &last_time);

    while(!shutdown_requested){

        Period_markEvent(PERIOD_EVENT_SAMPLE_LIGHT);
        sample = lightSensor_read();

        // check for dip
        if (!currently_dipped && sample < (average - check_for_dip)){
            currently_dipped = true;
            current_dips++;
        }
        else if(currently_dipped && sample > (last_sample + hysteresis)){
            currently_dipped = false;
        }

        last_sample = sample;

        // calculate average
        if (average_init == false){
            average = sample;
            average_init = true;
        }
        else{
            average = (smoothing * average) + ((1.0 - smoothing) * sample);
        }


        
        // make sure arrays won't overflow
        if(current_size + 1 > buffer_size){
            buffer_size *=2;
        
            pthread_mutex_lock(&history_mutex);
            double *new_history = (double *)realloc(history, buffer_size * sizeof(double));
            
            double *new_current_data = (double *)realloc(current_data, buffer_size * sizeof(double));
            // Check if realloc succeeded
            if (new_current_data == NULL) {
                fprintf(stderr, "Memory reallocation failed!\n");
                
            }
        
            // Update the pointer and buffer size
            current_data = new_current_data;
            history = new_history;
            pthread_mutex_unlock(&history_mutex);
        }

        // track current data
        current_data[current_size] = sample;
        current_size++;
        totalCount++;
        
        clock_gettime(CLOCK_MONOTONIC, &current_time);
        double elapsed = (current_time.tv_sec - last_time.tv_sec) +
                         (current_time.tv_nsec - last_time.tv_nsec) / 1e9;

        if (elapsed >= 1.0) {
            Sampler_moveCurrentDataToHistory();
            last_time = current_time;  // Reset timer
        }

        MyNanoSleep(1.0);
    }
    return NULL;
}