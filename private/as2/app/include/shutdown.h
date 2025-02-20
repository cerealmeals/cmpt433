#ifndef SHUTDOWN_H
#define SHUTDOWN_H
void shutdown_init(void);
void shutdown_shutdown(void);
void shutdown_waitForShutdown(void);
void shutdown_cleanup(void);

#endif