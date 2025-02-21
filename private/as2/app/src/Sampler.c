#include "Sampler.h"
#include "lightSensor.h"
#include "pthread.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdatomic.h>
#include <signal.h>
#include <time.h>


static long long totalCount = 0;
static int current_size = 0;
static int history_size = 0;
static pthread_t thread;
static volatile sig_atomic_t shutdown_requested = 0;

static void * thread_function(void* arg);

void Sampler_init(void)
{
    if (pthread_create(&thread, NULL, thread_function, NULL) != 0) {
        perror("Failed to create thread\n");
        exit(EXIT_FAILURE);
    }
}

void Sampler_cleanup(void)
{
    pthread_kill(thread, SIGUSR1);
    printf("before join in Sampler\n");
    int ret = pthread_join(thread, NULL);
    
    if (ret != 0) {
        perror("pthread_join failed");
        exit(EXIT_FAILURE);
    }
    printf("after join in Sampler\n");
}

void Sampler_moveCurrentDataToHistory(void)
{
    history_size = current_size;
    current_size = 0;
    printf("history size: %d\n", history_size);
}

int Sampler_getHistorySize(void)
{
    return 1;
}

// double* Sampler_getHistory(int *size)
// {
//     double to_return = 2.0 * (*size);
//     return &to_return;
// }

double Sampler_getAverageReading(void)
{
    return 1.0;
}


long long Sampler_getNumSamplesTaken(void)
{
    return totalCount;
}

static void signal_handler(int sig) {
    if (sig == SIGUSR1) {
        printf("Received SIGUSR1 in thread, shutting down...\n");
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
        perror("Arg should be something");
        return NULL;
    }

    signal(SIGUSR1, signal_handler);

    struct timespec last_time, current_time;
    //int sample;
    clock_gettime(CLOCK_MONOTONIC, &last_time);

    while(!shutdown_requested){

        //sample = lightSensor_read();

        totalCount++;
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