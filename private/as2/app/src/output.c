#include "output.h"

#include "periodTimer.h"
#include "Sampler.h"
#include "UseLCD.h"
#include "RotaryEncoder.h"
#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <stdbool.h>
#define NUMBER_OF_SAMPLES_IN_OUTPUT 10
#define MAX_MESSAGE 1024

static pthread_t output_thread;
static bool is_init = false;
static volatile sig_atomic_t shutdown_requested = 0;

static void * thread_function(void* arg);

void Output_init(void)
{
    assert(!is_init);
    UseLCD_init();
    if (pthread_create(&output_thread, NULL, thread_function, NULL) != 0){
        perror("Failed to create thread in network_listener\n");
        exit(EXIT_FAILURE);
    }

    is_init = true;
}

void Output_cleanup(void)
{
    assert(is_init);
    
    shutdown_requested = 1;
    //printf("output shutdown requested\n");
    
    if (pthread_join(output_thread, NULL) != 0) {
        perror("pthread_join failed");
        exit(EXIT_FAILURE);
    }
    UseLCD_cleanup();
    is_init = false;
}

static void MyMicroSleep(double mSeconds)
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

    while(!shutdown_requested){

        Period_statistics_t stats;
        int flashs = RotartEncoder_currentVolume();
        int history_size = Sampler_getHistorySize();
        double * history = Sampler_getHistory(&history_size);
        double average = Sampler_getAverageReading();
        int history_dips = Sampler_getdips();
        Period_getStatisticsAndClear(PERIOD_EVENT_SAMPLE_LIGHT, &stats);
        printf("#Smpl/s = %d Flash @ %dHz avg = %.3fV dips = %d Smpl ms[ %.3f, %.3f] avg %.3f/%d\n",
            history_size, flashs, average, history_dips, stats.minPeriodInMs, stats.maxPeriodInMs, stats.avgPeriodInMs, stats.numSamples);
        
        int offset = history_size / NUMBER_OF_SAMPLES_IN_OUTPUT;
        for(int i = 0; i < NUMBER_OF_SAMPLES_IN_OUTPUT; i++){
            printf(" %d:%.3f", i, history[i*offset]);
        }
        printf("\n");
        char message[MAX_MESSAGE];
        snprintf(message, MAX_MESSAGE, "Sam N.\nFalsh @  %dHz\nDips =  %d\nMax ms:  %.1f", flashs, history_dips, stats.maxPeriodInMs);
        UseLCD_updateScreen(message);
        free(history);
        
        MyMicroSleep(1000.0);
    }
    return NULL;
}