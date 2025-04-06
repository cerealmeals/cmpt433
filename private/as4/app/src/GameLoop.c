#include "GameLoop.h"
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>  // For usleep(), sleep()
#include "Button_statemachine.h"
#include "JoystickButton.h"  // The HAL with JoystickButton_init/cleanup + callback typedef
#include "shutdown.h"         // For shutdown_shutdown()
#include "Watch_gpio.h"
#include "LCD.h"
#include "AccelerometerApp.h"
#include "NeopixelApp.h"

#define DELAY_ON_ROTARY_BUTTON 100000  // 0.1 second delay

// Buffers for formatted strings
static char screen_buffer[128];

static int misses = 0;
static int hits = 0;

static bool Rotary_Button_busy = false;  // Prevents multiple button triggers

static bool s_isInit = false;

// Thread controls for updating the LCD every second
static pthread_t s_lcdThread;
static bool s_lcdThreadShouldExit = false;

// Forward declarations
static void joystickButtonCallback(int counter);
static void* button_handler(void* arg);
static void RotaryButtonCallback(int counter);
static void HandleShoot(void);
static void PushToLCD(void);

//---------------------------------------------------------------------------
// The LCD update thread: calls PushToLCD() every second
//---------------------------------------------------------------------------
static void* lcdUpdateThread(void* arg)
{
    (void)arg;
    while (!s_lcdThreadShouldExit) {
        PushToLCD();       // Refresh the LCD
        sleep(1);          // Wait 1 second
    }
    return NULL;
}

void GameLoop_init(void)
{
    assert(!s_isInit);
    srand((unsigned)time(NULL));
    int randomValueX = (rand() % 1001) - 500;
    int randomValueY = (rand() % 1001) - 500;
    
    LCD_init();

    // Initialize the joystick button HAL, giving it our callback
    JoystickButton_init(joystickButtonCallback);
    Button_statemachine_init(RotaryButtonCallback);
    Watch_gpio_Start_Watching();
    AccelerometerApp_SetTarget(randomValueX, randomValueY);

    // Create the LCD update thread
    s_lcdThreadShouldExit = false;
    if (pthread_create(&s_lcdThread, NULL, lcdUpdateThread, NULL) != 0) {
        perror("GameLoop_init: Could not create lcdUpdateThread");
        exit(EXIT_FAILURE);
    }

    s_isInit = true;
}

void GameLoop_cleanup(void)
{
    assert(s_isInit);

    // Signal LCD update thread to stop
    s_lcdThreadShouldExit = true;
    pthread_join(s_lcdThread, NULL);

    // Cleanup the joystick button HAL
    JoystickButton_cleanup();
    Button_statemachine_cleanup();
    Watch_gpio_cleanup();
    LCD_cleanup();

    s_isInit = false;
}



/**
 * @brief This is the callback function we pass to the JoystickButton HAL.
 *        Every time the button is pressed, the HAL calls this.
 *
 * @param halCounter The press count from the HAL, if provided. (We won't rely on it.)
 */
static void joystickButtonCallback(int counter)
{
    static int last_counter = 0;
    if (counter == last_counter) {
        //printf("same as last time button\n");
        return;
    } else {
        last_counter = counter;
    }
    // Call the shutdown mechanism to unlock any waiters (shutdown_waitForShutdown() etc.)
    shutdown_shutdown();
}

//---------------------------------------------------------------------------
// This thread re-enables the rotary button after 0.1s, preventing fast double-taps
//---------------------------------------------------------------------------
static void* button_handler(void* arg)
{
    if (arg != NULL) {
        printf("Button sleep thread should not have arguments\n");
        return NULL;
    }

    usleep(DELAY_ON_ROTARY_BUTTON); 
    // re-enable the button
    Rotary_Button_busy = false;
    return NULL;
}

//---------------------------------------------------------------------------
// Rotary button callback from the Button_statemachine HAL
//---------------------------------------------------------------------------
static void RotaryButtonCallback(int counter)
{
    static int last_counter = 0;
    if (counter == last_counter){
        return;
    } else {
        last_counter = counter;
    }

    if (Rotary_Button_busy) {
        return;
    }

    Rotary_Button_busy = true;  // Mark as busy
    HandleShoot();

    pthread_t thread;
    pthread_create(&thread, NULL, button_handler, NULL);
    pthread_detach(thread);  // Let the thread clean itself up
}

//---------------------------------------------------------------------------
// Called whenever the user "shoots" (the rotary button callback).
// We check if the accelerometer is on-target, increment hits/misses, 
// choose a new random target if it was a hit, and update the LCD once.
//---------------------------------------------------------------------------
static void HandleShoot(void)
{
    bool hit = AccelerometerApp_IsOnTarget();
    AccelerometerApp_SetControlNeoPixel(false);
    if(hit) {
        int randomValueX = (rand() % 1001) - 500;
        int randomValueY = (rand() % 1001) - 500;
        AccelerometerApp_SetTarget(randomValueX, randomValueY);
        hits++;
        NeopixelApp_HitAnimation();
    } else {
        misses++;
        NeopixelApp_MissAnimation();
    }
    AccelerometerApp_SetControlNeoPixel(true);
    // We *also* update the LCD right away (in addition to the once-per-second thread)
    PushToLCD();
}

//---------------------------------------------------------------------------
// PushToLCD: prints hits, misses, and elapsed time on the LCD + console
//---------------------------------------------------------------------------
static void PushToLCD(void)
{
    static time_t startTime = 0;

    // If this is the first time calling, set startTime
    if (startTime == 0) {
        startTime = time(NULL);
    }

    time_t now = time(NULL);
    int elapsed = (int)(now - startTime);    
    int minutes = elapsed / 60;
    int seconds = elapsed % 60;

    snprintf(screen_buffer, sizeof(screen_buffer),
             "Hits: %d\nMisses: %d\nTime: %02d:%02d",
             hits, misses, minutes, seconds);

    LCD_updateScreen(screen_buffer);
}
