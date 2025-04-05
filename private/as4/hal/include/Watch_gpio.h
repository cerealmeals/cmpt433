#ifndef WATCH_GPIO_H
#define WATCH_GPIO_H

#include <stdbool.h>
#include "gpio.h"

// Define a function pointer type for GPIO callbacks
typedef void (*gpio_callback_t)(bool isRising);



// void Watch_gpio_init();

void Watch_gpio_cleanup();

int Watch_gpio_Add_GPIO_Line(enum eGpioChips chip, int line, gpio_callback_t callback);

void Watch_gpio_Start_Watching();

#endif