#include "gpio_events.h"
#include "Watch_gpio.h"
#include "Rotary_Encoder_statemachine.h"
#include "Button_statemachine.h"
#include "shutdown.h"
#include "audio_handler.h"
#include "output.h"
#include "JoystickButton.h"
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>  // For usleep()
#include <assert.h>

#define DELAY_ON_BUTTON 500000  // 0.5 second delay
#define DELAY_ON_JOYSTICK_BUTTON 1500000

static int Rotary_encoder_last_count = 0;
static int button_count = 0;
static bool Button_busy = false;  // Prevents multiple button triggers

static bool Joystick_button_busy = false;

static bool is_init = false;

static void* button_handler(void* arg)
{
    if(arg != NULL){
        printf("Button sleep Thread should not have arguments\n");
        return NULL;
    }

    usleep(DELAY_ON_BUTTON);

    //printf("Button is ready for new presses.\n");
    Button_busy = false;  // Re-enable button
    return NULL;
}


static void button(int counter)
{
    static int last_counter = 0;
    if (counter == last_counter){
        //printf("same as last time button\n");
        return;
    }
    else{
        last_counter = counter;
    }
    if (Button_busy) {
        //printf("Button is busy, ignoring new press.\n");
        return;
    }

    Button_busy = true;  // Mark as busy
    button_count++;
    audio_handler_cycleBeatMode();
    pthread_t thread;
    //printf("Button pressed, counter: %d\n",button_count);
    if(button_count == 3){
        //printf("Check?\n");
        shutdown_shutdown();
    }

    pthread_create(&thread, NULL, button_handler, NULL);
    pthread_detach(thread);  // Let the thread clean itself up
}

static void Rotation_handleChange(int counter)
{
    if(Rotary_encoder_last_count > counter){
        audio_handler_adjustBPM(true);
    }
    else if(Rotary_encoder_last_count < counter){
        audio_handler_adjustBPM(false);
    }
    Rotary_encoder_last_count = counter;
}

static void* Joystick_button_handler(void* arg)
{
    if(arg != NULL){
        printf("Button sleep Thread should not have arguments\n");
        return NULL;
    }

    usleep(DELAY_ON_JOYSTICK_BUTTON);

    //printf("Button is ready for new presses.\n");
    Joystick_button_busy = false;  // Re-enable button
    return NULL;
}

static void joystick_button_callback(int counter) {
    static int last_counter = 0;
    if (counter == last_counter){
        //printf("same as last time button\n");
        return;
    }
    else{
        last_counter = counter;
        Output_cycle_LCD_mode();
        //printf("LCD stuff\n");
    }
    if (Joystick_button_busy) {
        //printf("Button is busy, ignoring new press.\n");
        return;
    }
    Joystick_button_busy = true;  // Mark as busy
    pthread_t thread;
    pthread_create(&thread, NULL, Joystick_button_handler, NULL);
    pthread_detach(thread);  // Let the thread clean itself up
}

void gpio_events_init()
{
    assert(!is_init);
    Rotary_Encoder_statemachine_init(Rotation_handleChange);
    Button_statemachine_init(button);
    JoystickButton_init(joystick_button_callback);
    Watch_gpio_Start_Watching();
    is_init = true;
}

void gpio_events_cleanup()
{
    assert(is_init);
    JoystickButton_cleanup();
    Rotary_Encoder_statemachine_cleanup();
    Button_statemachine_cleanup();
    Watch_gpio_cleanup();
    is_init = false;
}
