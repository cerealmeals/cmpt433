// Sample state machine for one GPIO pin.

#include "gpio.h"

#include"RE_statemachine.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdatomic.h>
#include <pthread.h>
#include <signal.h>

// Pin config info: GPIO 24 (Rotary Encoder PUSH)
//   $ gpiofind GPIO24
//   >> gpiochip0 10
// #define GPIO_CHIP_BTN           GPIO_CHIP_0
// #define GPIO_LINE_NUMBER_BTN    10

#define GPIO_CHIP_A             GPIO_CHIP_2
#define GPIO_LINE_NUMBER_A      7
#define GPIO_CHIP_B             GPIO_CHIP_2
#define GPIO_LINE_NUMBER_B      8

#define NUMLINES 2


static bool isInitialized = false;


struct GpioLine* lines[NUMLINES] = {NULL, NULL};

static atomic_int counter = 0;
static atomic_bool track_CW = false;
static atomic_bool track_CCW = false;
static pthread_t State_thread;

static volatile sig_atomic_t shutdown_requested = 0;

static void * thread_function(void* arg);

/*
    Define the Statemachine Data Structures
*/
struct Rotation_stateEvent {
    struct state* pNextState;
    void (*action)();
};
struct state {
    struct Rotation_stateEvent A_rising;
    struct Rotation_stateEvent A_falling;
    struct Rotation_stateEvent B_falling;
    struct Rotation_stateEvent B_rising;
};


/*
    START STATEMACHINE
*/
static void start_CW(void)
{
    track_CW = true;
}

static void start_CCW(void)
{
    track_CCW = true;
}

static void end_CW(void)
{
    track_CCW = false;
    if (track_CW == true){
        counter++;
    }
    track_CW = false;
}

static void end_CCW(void)
{
    track_CW = false;
    if (track_CCW == true){
        counter--;
    }
    track_CCW = false;
}

struct state states[] = {
    { // rest = 0
        .A_rising = {&states[0], NULL},
        .A_falling = {&states[1], start_CW},
        .B_rising = {&states[0], NULL},
        .B_falling = {&states[3], start_CCW},
    },

    { // 1
        .A_rising = {&states[0], end_CCW},
        .A_falling = {&states[1], NULL},
        .B_rising = {&states[1], NULL},
        .B_falling = {&states[2], NULL},
    },

    { // 2
        .A_rising = {&states[3], NULL},
        .A_falling = {&states[2], NULL},
        .B_rising = {&states[1], NULL},
        .B_falling = {&states[2], NULL},
    },

    { // 3
        .A_rising = {&states[3], NULL},
        .A_falling = {&states[2], NULL},
        .B_rising = {&states[0], end_CW},
        .B_falling = {&states[3], NULL},
    },
};
/*
    END STATEMACHINE
*/

struct state* pCurrentState = &states[0];



typedef struct {
    StateMachineCallback callback;
} ThreadData;


void RE_StateMachine_init()
{
    assert(!isInitialized);
    lines[0] = Gpio_openForEvents(GPIO_CHIP_A, GPIO_LINE_NUMBER_A);
    lines[1] = Gpio_openForEvents(GPIO_CHIP_B, GPIO_LINE_NUMBER_B);
    isInitialized = true;
}
void RE_StateMachine_cleanup()
{
    assert(isInitialized);
    
    if (pthread_kill(State_thread, SIGUSR1) != 0) {
        perror("pthread_kill failed");
        exit(EXIT_FAILURE);
    }
    //printf("State machine shutdown requested\n");
    
    if (pthread_join(State_thread, NULL) != 0) {
        perror("pthread_join failed");
        exit(EXIT_FAILURE);
    }
    Gpio_close(lines[0]);
    Gpio_close(lines[1]);

    isInitialized = false;
}

int RE_StateMachine_getValue()
{
    return counter;
}

int RE_StateMachine_doState(StateMachineCallback callback)
{
    assert(isInitialized);

    // Create an instance of ThreadData
    ThreadData* threadData = (ThreadData*)malloc(sizeof(ThreadData));
    if (threadData == NULL) {
        perror("Failed to allocate memory for thread data\n");
        return 1;
    }

    threadData->callback = callback;

    // Create thread and pass the callback
    if (pthread_create(&State_thread, NULL, thread_function, (void*)threadData) != 0) {
        perror("Failed to create thread\n");
        return 1;
    }
    return 0;
}

static void State_signal_handler(int sig) {
    if (sig == SIGUSR1) {
        //printf("Received SIGUSR1 in Output, shutting down...\n");
        shutdown_requested = 1;
    }
}

static void * thread_function(void* arg)
{
    if(arg == NULL){
        perror("Arg should be something");
        return NULL;
    }

    ThreadData* data = (ThreadData*)arg;

    // Cast the argument back to the function pointer type
    StateMachineCallback callback = data->callback;
    free(data);

    signal(SIGUSR1, State_signal_handler);

    while (!shutdown_requested) {
        struct gpiod_line_bulk bulkEvents;
        int numEvents = Gpio_waitForLineChange(lines, NUMLINES, &bulkEvents);

        // Iterate over the event
        for (int i = 0; i < numEvents; i++)
        {
            // Get the line handle for this event
            struct gpiod_line *line_handle = gpiod_line_bulk_get_line(&bulkEvents, i);

            // Get the number of this line
            unsigned int this_line_number = gpiod_line_offset(line_handle);

            // Get the line event
            struct gpiod_line_event event;
            if (gpiod_line_event_read(line_handle,&event) == -1) {
                perror("Line Event");
                exit(EXIT_FAILURE);
            }


            // Run the state machine
            bool isRising = event.event_type == GPIOD_LINE_EVENT_RISING_EDGE;

            // Can check with line it is, if you have more than one...
            bool line_A = this_line_number == GPIO_LINE_NUMBER_A;
            struct Rotation_stateEvent* pStateEvent = NULL;
            if (line_A)
            {
                if (isRising) {
                    pStateEvent = &pCurrentState->A_rising;
                } else {
                    pStateEvent = &pCurrentState->A_falling;
                } 
            }
            else{
                if (isRising) {
                    pStateEvent = &pCurrentState->B_rising;
                } else {
                    pStateEvent = &pCurrentState->B_falling;
                }
            }
            
            // Do the action
            if (pStateEvent->action != NULL) {
                pStateEvent->action();
            }
            pCurrentState = pStateEvent->pNextState;

            

            // DEBUG INFO ABOUT STATEMACHINE
            #if 0
            int newState = (pCurrentState - &states[0]);
            double time = event.ts.tv_sec + event.ts.tv_nsec / 1000000000.0;
            printf("State machine Debug: i=%d/%d  line num/dir = %d %8s -> new state %d     [%f]\n", 
                i, 
                numEvents,
                this_line_number, 
                isRising ? "RISING": "falling", 
                newState,
                time);
            #endif


            // run given function with counter
            callback(counter);
        }
    }
    return NULL;
}
