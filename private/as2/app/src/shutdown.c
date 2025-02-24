#include "shutdown.h"
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <assert.h>

// Declare a mutex
static pthread_mutex_t shutdown_mutex;
static bool is_init = false;

void shutdown_init(void)
{
    assert(!is_init);
    // Initialize the mutex
    pthread_mutex_init(&shutdown_mutex, NULL);
    // Lock the mutex
    pthread_mutex_lock(&shutdown_mutex);
    is_init = true;
}

void shutdown_shutdown(void)
{
    assert(is_init == true);
    // Unlock the mutex letting wait for shutdown pass through
    pthread_mutex_unlock(&shutdown_mutex);
}

void shutdown_waitForShutdown(void)
{
    assert(is_init == true);
    // Lock the mutex making this wait for the shutdown call
    pthread_mutex_lock(&shutdown_mutex);
}

void shutdown_cleanup(void)
{
    assert(is_init == true);
    // Destroy the mutex
    pthread_mutex_destroy(&shutdown_mutex);
    is_init = false;
}