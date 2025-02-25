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
#define SIGNAL_NUMBER 182346
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
    UseLCD_cleanup();
    if (pthread_kill(output_thread, SIGNAL_NUMBER) != 0) {
        perror("pthread_kill failed");
        exit(EXIT_FAILURE);
    }
    
    if (pthread_join(output_thread, NULL) != 0) {
        perror("pthread_join failed");
        exit(EXIT_FAILURE);
    }

    is_init = false;
}

static void Output_signal_handler(int sig) {
    if (sig == SIGNAL_NUMBER) {
        printf("Received SIGNAL_NUMBER in Output, shutting down...\n");
        shutdown_requested = 1;
    }
}

static void * thread_function(void* arg)
{
    if(arg != NULL){
        perror("Arg should be something\n");
        return NULL;
    }

    signal(SIGUSR1, Output_signal_handler);

    struct timespec last_time, current_time;
    clock_gettime(CLOCK_MONOTONIC, &last_time);

    while(!shutdown_requested){

        clock_gettime(CLOCK_MONOTONIC, &current_time);
        double elapsed = (current_time.tv_sec - last_time.tv_sec) +
                         (current_time.tv_nsec - last_time.tv_nsec) / 1e9;

        if (elapsed >= 1.0) {

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
            last_time = current_time;  // Reset timer
        }
    }
    return NULL;
}