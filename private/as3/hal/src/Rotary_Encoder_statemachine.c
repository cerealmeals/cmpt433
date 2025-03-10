#include "Rotary_Encoder_statemachine.h"
#include "Watch_gpio.h"
#include "gpio.h"
#include <stdatomic.h>
#include <assert.h>
#include <stdbool.h>

#define GPIO_CHIP_button          GPIO_CHIP_0
#define GPIO_LINE_NUMBER_button   10
#define GPIO_CHIP_A             GPIO_CHIP_2
#define GPIO_LINE_NUMBER_A      7
#define GPIO_CHIP_B             GPIO_CHIP_2
#define GPIO_LINE_NUMBER_B      8

static atomic_int Rotation_counter = 0;
static atomic_int Button_counter = 0;
static atomic_bool track_CW = false;
static atomic_bool track_CCW = false;

// static void LineA_callback(bool isRising);
static void LineB_callback(bool isRising);
static void Button_line_callback(bool isRising);


static Rotary_Encoder_callback Rotation_callback_function = NULL;
static Rotary_Encoder_callback Button_callback_function = NULL;
static bool is_init = false;

void Rotary_Encoder_statemachine_init(Rotary_Encoder_callback Roation_callback, Rotary_Encoder_callback Button_callback)
{
    assert(!is_init);
    Rotation_callback_function = Roation_callback;
    Button_callback_function = Button_callback;
    // Watch_gpio_Add_GPIO_Line(GPIO_CHIP_A, GPIO_LINE_NUMBER_A, LineA_callback);
    Watch_gpio_Add_GPIO_Line(GPIO_CHIP_B, GPIO_LINE_NUMBER_B, LineB_callback);
    Watch_gpio_Add_GPIO_Line(GPIO_CHIP_button, GPIO_LINE_NUMBER_button, Button_line_callback);
    is_init = false;
}

void Rotary_Encoder_statemachine_cleanup()
{
    assert(is_init);
    Rotation_callback_function = NULL;
    Button_callback_function = NULL;
    is_init = true;
}

/*
    Define the Statemachine Data Structures for rotation
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

struct Rotation_state Rotatrion_states[] = {
    { // rest = 0
        .A_rising = {&Rotatrion_states[0], NULL},
        .A_falling = {&Rotatrion_states[1], start_CCW},
        .B_rising = {&Rotatrion_states[0], NULL},
        .B_falling = {&Rotatrion_states[3], start_CW},
    },

    { // 1
        .A_rising = {&Rotatrion_states[0], end_CW},
        .A_falling = {&Rotatrion_states[1], NULL},
        .B_rising = {&Rotatrion_states[1], NULL},
        .B_falling = {&Rotatrion_states[2], NULL},
    },

    { // 2
        .A_rising = {&Rotatrion_states[3], NULL},
        .A_falling = {&Rotatrion_states[2], NULL},
        .B_rising = {&Rotatrion_states[1], NULL},
        .B_falling = {&Rotatrion_states[2], NULL},
    },

    { // 3
        .A_rising = {&Rotatrion_states[3], NULL},
        .A_falling = {&Rotatrion_states[2], NULL},
        .B_rising = {&Rotatrion_states[0], end_CCW},
        .B_falling = {&Rotatrion_states[3], NULL},
    },
};
/*
    END STATEMACHINE
*/



/*
    Define the Statemachine Data Structures for button
*/
struct button_stateEvent {
    struct button_state* pNextState;
    void (*action)();
};
struct button_state {
    struct button_stateEvent rising;
    struct button_stateEvent falling;
};


/*
    START STATEMACHINE
*/
static void on_release(void)
{
    Button_counter++;
}

struct button_state Button_states[] = {
    { // Not pressed
        .rising = {&Button_states[0], NULL},
        .falling = {&Button_states[1], NULL},
    },

    { // Pressed
        .rising = {&Button_states[0], on_release},
        .falling = {&Button_states[1], NULL},
    },
};
/*
    END STATEMACHINE
*/


struct button_state* Button_CurrentState = &Button_states[0];
struct Rotation_state* Rotation_CurrentState = &Rotatrion_states[0];

// static void LineA_callback(bool isRising)
// {
//     struct Rotation_stateEvent* pStateEvent = NULL;
//     if (isRising) {
//         pStateEvent = &Rotation_CurrentState->A_rising;
//     } else {
//         pStateEvent = &Rotation_CurrentState->A_falling;
//     }

//     // Do the action
//     if (pStateEvent->action != NULL) {
//         pStateEvent->action();
//     }
//     Rotation_CurrentState = pStateEvent->pNextState;

//     Rotation_callback_function(Rotation_counter);
// }

static void LineB_callback(bool isRising)
{
    struct Rotation_stateEvent* pStateEvent = NULL;
    if (isRising) {
        pStateEvent = &Rotation_CurrentState->B_rising;
    } else {
        pStateEvent = &Rotation_CurrentState->B_falling;
    }

    // Do the action
    if (pStateEvent->action != NULL) {
        pStateEvent->action();
    }
    Rotation_CurrentState = pStateEvent->pNextState;

    Rotation_callback_function(Rotation_counter);
}



static void Button_line_callback(bool isRising)
{
    struct button_stateEvent* pStateEvent = NULL;
    if (isRising) {
        pStateEvent = &Button_CurrentState->rising;
    } else {
        pStateEvent = &Button_CurrentState->falling;
    } 

    // Do the action
    if (pStateEvent->action != NULL) {
        pStateEvent->action();
    }
    Button_CurrentState = pStateEvent->pNextState;

    Button_callback_function(Button_counter);
}