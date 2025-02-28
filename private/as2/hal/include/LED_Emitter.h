// This module controls an LED emitter by configuring its PWM (Pulse Width Modulation)
// settings, such as duty cycle, period, and frequency. 
// It provides an interface to initialize, configure, and clean up the LED emitter.

typedef enum {
    GPIO12 = 0, 
    NUM_PINS
} GPIOPINS;


// Initializes the LED emitter by setting up the PWM period, duty cycle, and enabling the GPIO pin.
int LED_Emitter_init(void);

// Sets the duty cycle of the PWM signal for the specified GPIO pin, controlling the brightness of the LED.
int LED_Emitter_setDutyCycle(float percent_on, GPIOPINS pin);

// Sets the period of the PWM signal for the specified GPIO pin, determining the frequency of the LED blinking.
int LED_Emitter_setPeriod(long long new_period, GPIOPINS pin);

// Configures the LED to blink a specified number of times per second by adjusting the PWM period and duty cycle.
int LED_Emitter_setTimesPerSecond(int times_per_second, GPIOPINS pin);

// Disables the LED emitter and cleans up resources.
int LED_Emitter_cleanup(void);