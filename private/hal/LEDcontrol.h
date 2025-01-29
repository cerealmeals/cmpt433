#include <stdbool.h>
#ifndef LEDCONTROL_H
#define LEDCONTROL_H

typedef enum {
    RED = 0,
    GREEN
} LED_COLOUR;


void LEDcontrol_init(void);

void LEDcontrol_on(LED_COLOUR colour);

void LEDcontrol_off(LED_COLOUR colour);

void LEDcontrol_timer(LED_COLOUR colour, int on_time, int off_time);

void LEDcontrol_heartbeat(LED_COLOUR colour, bool invert);

// char* LEDcontrol_trigger(LED_COLOUR colour);

// bool LEDcontrol_status(LED_COLOUR colour);





#endif 