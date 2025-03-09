#include "Rotary_Encoder_statemachine.h"
#include "Watch_gpio.h"
#include "gpio.h"
#include <stdatomic.h>
#include <assert.h>
#include <stdbool.h>

#define GPIO_CHIP_A             GPIO_CHIP_2
#define GPIO_LINE_NUMBER_A      7
#define GPIO_CHIP_B             GPIO_CHIP_2
#define GPIO_LINE_NUMBER_B      8

static atomic_int Rotation_counter = 0;
static atomic_bool track_CW = false;
static atomic_bool track_CCW = false;

static void LineA_callback(bool isRising);
static void LineB_callback(bool isRising);

static Rotary_Encoder_callback callback_function = NULL;
static bool is_init = false;

void Rotary_Encoder_statemachine_init(Rotary_Encoder_callback callback)
{
    assert(!is_init);
    callback_function = callback;
    Watch_gpio_Add_GPIO_Line(GPIO_CHIP_A, GPIO_LINE_NUMBER_A, LineA_callback);
    Watch_gpio_Add_GPIO_Line(GPIO_CHIP_B, GPIO_LINE_NUMBER_B, LineB_callback);
    is_init = false;
}

void Rotary_Encoder_statemachine_cleanup()
{
    assert(is_init);
    callback_function = NULL
    is_init = true;
}

/*
    Define the Statemachine Data Structures
*/
struct Rotation_stateEvent {
    struct Rotation_state* pNextState;
    void (*action)();
};
struct Rotation_state {
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
        Rotation_counter++;
    }
    track_CW = false;
}

static void end_CCW(void)
{
    track_CW = false;
    if (track_CCW == true){
        Rotation_counter--;
    }
    track_CCW = false;
}

struct Rotation_state states[] = {
    { // rest = 0
        .A_rising = {&states[0], NULL},
        .A_falling = {&states[1], start_CCW},
        .B_rising = {&states[0], NULL},
        .B_falling = {&states[3], start_CW},
    },

    { // 1
        .A_rising = {&states[0], end_CW},
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
        .B_rising = {&states[0], end_CCW},
        .B_falling = {&states[3], NULL},
    },
};
/*
    END STATEMACHINE
*/

struct Rotation_state* pCurrentState = &states[0];

static void LineA_callback(bool isRising)
{
    struct Rotation_stateEvent* pStateEvent = NULL;
    if (isRising) {
        pStateEvent = &pCurrentState->A_rising;
    } else {
        pStateEvent = &pCurrentState->A_falling;
    }

    // Do the action
    if (pStateEvent->action != NULL) {
        pStateEvent->action();
    }
    pCurrentState = pStateEvent->pNextState;

    callback_function(Rotation_counter);
}

static void LineB_callback(bool isRising)
{
    struct Rotation_stateEvent* pStateEvent = NULL;
    if (isRising) {
        pStateEvent = &pCurrentState->B_rising;
    } else {
        pStateEvent = &pCurrentState->B_falling;
    }

    // Do the action
    if (pStateEvent->action != NULL) {
        pStateEvent->action();
    }
    pCurrentState = pStateEvent->pNextState;

    callback_function(Rotation_counter);
}