#include "Sampler.h"
#include "lightSensor.h"
#include "pthread.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdatomic.h>
#include <signal.h>
#include <time.h>
#include <stdbool.h>

static double average;
static bool average_init = false;
static double smoothing = 0.999;
static long long totalCount = 0;
static int current_size = 0;
static int history_size = 0;
static int buffer_size = 500;
static double * history;
static double * current_data;

static pthread_t thread;
static volatile sig_atomic_t shutdown_requested = 0;
static bool is_init = false;

static void * thread_function(void* arg);

void Sampler_init(void)
{
    assert(!is_init);
    lightSensor_init();
    if (pthread_create(&thread, NULL, thread_function, NULL) != 0) {
        perror("Failed to create thread\n");
        exit(EXIT_FAILURE);
    }
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

    is_init = true;
}

void Sampler_cleanup(void)
{
    assert(is_init);

    if (pthread_kill(thread, SIGUSR1) != 0) {
        perror("pthread_kill failed");
    }
    
    if (pthread_join(thread, NULL) != 0) {
        perror("pthread_join failed");
        exit(EXIT_FAILURE);
    }

    if (history != NULL) {
        free(history);
        history = NULL; // Avoid dangling pointer
    }

    if (current_data != NULL) {
        free(current_data);
        current_data = NULL; // Avoid dangling pointer
    }
    lightSensor_cleanup();
    is_init = false;
}

void Sampler_moveCurrentDataToHistory(void)
{
    assert(is_init);
    assert(current_size <= buffer_size);

    for(int i = 0; i < current_size; i++){
        history[i] = current_data[i];
    }
    
    history_size = current_size;
    current_size = 0;
    printf("History size: %d\n", history_size);
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

    for(int i = 0; i < return_size; i++){
        ret[i] = history[i];
    }
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

static void Sampler_signal_handler(int sig) {
    if (sig == SIGUSR1) {
        printf("Received SIGUSR1 in Sampler, shutting down...\n");
        shutdown_requested = 1;
    }
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
    else{
        printf("just checking\n");
    }

    signal(SIGUSR1, Sampler_signal_handler);

    struct timespec last_time, current_time;
    int sample;
    clock_gettime(CLOCK_MONOTONIC, &last_time);

    while(!shutdown_requested){

        sample = lightSensor_read();
        if (average_init == false){
            average = (double)sample;
        }
        else{
            average = (smoothing * average) + ((1.0 - smoothing) * (double)sample);
        }
        totalCount++;

        if(current_size + 1 > buffer_size){
            buffer_size *=2;
        
            double *new_history = (double *)realloc(history, buffer_size * sizeof(double));
            
            double *new_current_data = (double *)realloc(current_data, buffer_size * sizeof(double));
            // Check if realloc succeeded
            if (new_current_data == NULL) {
                fprintf(stderr, "Memory reallocation failed!\n");
                
            }
        
            // Update the pointer and buffer size
            current_data = new_current_data;
            history = new_history;
        }

        current_data[current_size] = (double)sample;
        current_size++;

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