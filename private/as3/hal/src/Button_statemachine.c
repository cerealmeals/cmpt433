#include "Button_statemachine.h"
#include "Watch_gpio.h"
#include "gpio.h"
#include <stdatomic.h>
#include <assert.h>
#include <stdbool.h>

#define GPIO_CHIP_button        GPIO_CHIP_0
#define GPIO_LINE_NUMBER_button 10

static atomic_int Button_counter = 0;
static void Button_line_callback(bool isRising);

static Button_callback Button_callback_function = NULL;
static bool is_init = false;

void Button_statemachine_init(Button_callback Button_callback)
{
    assert(!is_init);
    Button_callback_function = Button_callback;
    Watch_gpio_Add_GPIO_Line(GPIO_CHIP_button, GPIO_LINE_NUMBER_button, Button_line_callback);
    is_init = true;
}

void Button_statemachine_cleanup()
{
    assert(is_init);
    Button_callback_function = NULL;
    is_init = false;
}

struct button_stateEvent {
    struct button_state* pNextState;
    void (*action)();
};

struct button_state {
    struct button_stateEvent rising;
    struct button_stateEvent falling;
};

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

struct button_state* Button_CurrentState = &Button_states[0];

static void Button_line_callback(bool isRising)
{
    struct button_stateEvent* pStateEvent = isRising ? &Button_CurrentState->rising : &Button_CurrentState->falling;
    if (pStateEvent->action) {
        pStateEvent->action();
    }
    Button_CurrentState = pStateEvent->pNextState;
    Button_callback_function(Button_counter);
}
