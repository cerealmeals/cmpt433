#ifndef GAMELOOP_H
#define GAMELOOP_H

/**
 * @brief Initializes the joystick-based shutdown system.
 * 
 * Starts a background thread that periodically checks whether the joystick
 * button is pressed. If pressed, triggers shutdown via shutdown_shutdown().
 */
void GameLoop_init(void);

/**
 * @brief Cleans up resources used by the joystick-based shutdown system.
 * 
 * Stops the background thread that monitors the joystick.
 */
void GameLoop_cleanup(void);

#endif // GAMELOOP_H