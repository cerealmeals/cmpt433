

typedef enum {
    GPIO12 = 0, 
    NUM_PINS
} GPIOPINS;

int LED_Emitter_init(void);
int LED_Emitter_setDutyCycle(float percent_on, GPIOPINS pin);
int LED_Emitter_setPeriod(long long new_period, GPIOPINS pin);
int LED_Emitter_setTimesPerSecond(int times_per_second, GPIOPINS pin);
int LED_Emitter_cleanup(void);