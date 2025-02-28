// This module provides a mechanism to manage program shutdown in a controlled and thread-safe manner. 
// It uses a mutex to block threads until a shutdown signal is received, 
// ensuring orderly termination of the program.

#ifndef SHUTDOWN_H
#define SHUTDOWN_H

// Initializes the module, creates and locks a mutex to block shutdown waiting threads until a shutdown signal is received.
void shutdown_init(void);

// Unlocks the mutex, allowing any threads waiting for shutdown to proceed and terminate.
void shutdown_shutdown(void);

// Blocks the calling thread by locking the mutex, waiting for the shutdown signal to be triggered.
void shutdown_waitForShutdown(void);

// Cleans up resources by destroying the mutex and resetting the module's state.
void shutdown_cleanup(void);

#endif