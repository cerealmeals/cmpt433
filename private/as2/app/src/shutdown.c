#include "shutdown.h"
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <assert.h>

// Declare a mutex
static pthread_mutex_t mutex;
static bool is_init = false;

void shutdown_init(void)
{
    assert(!is_init);
    // Initialize the mutex
    pthread_mutex_init(&mutex, NULL);
    // Lock the mutex
    pthread_mutex_lock(&mutex);
    is_init = true;
}

void shutdown_shutdown(void)
{
    assert(is_init == true);
    // Unlock the mutex letting wait for shutdown pass through
    pthread_mutex_unlock(&mutex);
}

void shutdown_waitForShutdown(void)
{
    assert(is_init == true);
    // Lock the mutex making this wait for the shutdown call
    pthread_mutex_lock(&mutex);
}

void shutdown_cleanup(void)
{
    assert(is_init == true);
    // Destroy the mutex
    pthread_mutex_destroy(&mutex);
    is_init = false;
}