// Sample state machine for one GPIO pin.

#include "btn_statemachine.h"
#include "gpio.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdatomic.h>

// Pin config info: GPIO 24 (Rotary Encoder PUSH)
//   $ gpiofind GPIO24
//   >> gpiochip0 10
#define GPIO_CHIP_BTN           GPIO_CHIP_0
#define GPIO_LINE_NUMBER_BTN    10
#define GPIO_CHIP_A             GPIO_CHIP_2
#define GPIO_LINE_NUMBER_A      7
#define GPIO_CHIP_B             GPIO_CHIP_2
#define GPIO_LINE_NUMBER_B      8


static bool isInitialized = false;

struct GpioLine* s_lineBtn = NULL;
static atomic_int counter = 0;
static atomic_bool track_CW = false;
static atomic_bool track_CCW = false;

/*
    Define the Statemachine Data Structures
*/
struct stateEvent {
    struct state* pNextState;
    void (*action)();
};
struct state {
    struct stateEvent A_rising;
    struct stateEvent A_falling;
    struct stateEvent B_falling;
    struct stateEvent B_rising;
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
        .B_rising = (&states[0], NULL),
        .B_falling = (&states[3], start_CCW),
    },

    { // 1
        .A_rising = {&states[0], end_CCW},
        .A_falling = {&states[1], NULL},
        .B_rising = (&states[1], NULL),
        .B_falling = (&states[2], NULL),
    },

    { // 2
        .A_rising = {&states[3], NULL},
        .A_falling = {&states[2], NULL},
        .B_rising = (&states[1], NULL),
        .B_falling = (&states[2], NULL),
    },

    { // 3
        .A_rising = {&states[3], NULL},
        .A_falling = {&states[2], NULL},
        .B_rising = (&states[0], end_CW),
        .B_falling = (&states[3], NULL),
    },
};
/*
    END STATEMACHINE
*/

struct state* pCurrentState = &states[0];



void RE_StateMachine_init()
{
    assert(!isInitialized);
    s_lineBtn = Gpio_openForEvents(GPIO_CHIP, GPIO_LINE_NUMBER);
    isInitialized = true;
}
void RE_StateMachine_cleanup()
{
    assert(isInitialized);
    isInitialized = false;
    Gpio_close(s_lineBtn);
}

int RE_StateMachine_getValue()
{
    return counter;
}

// TODO: This should be on a background thread!
void RE_StateMachine_doState()
{
    assert(isInitialized);

    //printf("\n\nWaiting for an event...\n");
    // while (true) {
        struct gpiod_line_bulk bulkEvents;
        int numEvents = Gpio_waitForLineChange(s_lineBtn, &bulkEvents);

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
            bool isBtn = this_line_number == GPIO_LINE_NUMBER;
            assert (isBtn);

            struct stateEvent* pStateEvent = NULL;
            if (isRising) {
                pStateEvent = &pCurrentState->rising;
            } else {
                pStateEvent = &pCurrentState->falling;
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
        }
    // }

}
