#include "JoystickButton.h"
#include "Watch_gpio.h"
#include <assert.h>
#include <stdatomic.h>

#define GPIO_CHIP_BUTTON        GPIO_CHIP_2
#define GPIO_LINE_NUMBER_BUTTON 15

static atomic_int Button_counter = 0;
static JoystickButton_Callback Button_callback_function = NULL;

static void JoystickButton_line_callback(bool isRising);

struct Joystick_button_stateEvent {
    struct Joystick_button_state* pNextState;
    void (*action)();
};

struct Joystick_button_state {
    struct Joystick_button_stateEvent rising;
    struct Joystick_button_stateEvent falling;
};

static void on_release(void) {
    Button_counter++;
}

static struct Joystick_button_state Button_states[] = {
    { // Not pressed
        .rising = {&Button_states[0], NULL},
        .falling = {&Button_states[1], NULL},
    },
    { // Pressed
        .rising = {&Button_states[0], on_release},
        .falling = {&Button_states[1], NULL},
    },
};

static struct Joystick_button_state* Button_CurrentState = &Button_states[0];

static void JoystickButton_line_callback(bool isRising) {
    struct Joystick_button_stateEvent* pStateEvent = isRising ? &Button_CurrentState->rising : &Button_CurrentState->falling;
    if (pStateEvent->action) {
        pStateEvent->action();
    }
    Button_CurrentState = pStateEvent->pNextState;
    if (Button_callback_function) {
        Button_callback_function(Button_counter);
    }
}

void JoystickButton_init(JoystickButton_Callback callback) {
    assert(Button_callback_function == NULL);
    Button_callback_function = callback;
    Watch_gpio_Add_GPIO_Line(GPIO_CHIP_BUTTON, GPIO_LINE_NUMBER_BUTTON, JoystickButton_line_callback);
}

void JoystickButton_cleanup() {
    Button_callback_function = NULL;
}