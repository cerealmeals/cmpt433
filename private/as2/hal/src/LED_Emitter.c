#include "LED_Emitter.h"
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#define MAX 40
#define PATH_Length 20
#define NANO_SECOND_TO_SECOND 1000000000

typedef struct {
    char path[PATH_Length];
} PIN;

static PIN pins[NUM_PINS];
static long long period = 1000000;
static float duty_cycle = 0.5f;

static bool is_init = false;

static int Open_and_write(char* path, char* to_write);

int LED_Emitter_init(void)
{
    assert(!is_init);

    PIN gpio12 = {"/dev/hat/pwm/GPIO12"};
    pins[GPIO12] = gpio12;
    
    char path[MAX];
    snprintf(path, MAX, "%s%s", pins[GPIO12].path, "/period");
    
    char to_write[MAX];
    snprintf(to_write, MAX, "%lld", period);
    
    
    if (Open_and_write(path, to_write) < 0)
    {
        return -1;
    }

    
    snprintf(path, MAX, "%s%s", pins[GPIO12].path, "/duty_cycle");
    snprintf(to_write, MAX, "%f", duty_cycle);

    if (Open_and_write(path, to_write) < 0)
    {
        return -1;
    }

    // enable the light
    snprintf(path, MAX, "%s%s", pins[GPIO12].path, "/enable");
    snprintf(to_write, MAX, "1");

    if (Open_and_write(path, to_write) < 0)
    {
        return -1;
    }
    is_init = true;
    return 0;
}

int LED_Emitter_setDutyCycle(float percent_on, GPIOPINS pin)
{   
    assert(is_init);
    if (percent_on > 1 || percent_on < 0)
    {
        return -1;
    }
    duty_cycle = percent_on;

    char path[MAX];
    snprintf(path, MAX, "%s%s", pins[pin].path, "/duty_cycle");

    long long to_set = period * percent_on;
    char to_write[MAX];
    snprintf(to_write, MAX, "%lld", to_set);
    
    if (Open_and_write(path, to_write) < 0)
    {
        return -1;
    }

    return 0;
}
int LED_Emitter_setPeriod(long long new_period, GPIOPINS pin)
{
    assert(is_init);
    if (new_period < duty_cycle || new_period > 469754879)
    {
        printf("can't set the period to %lld\n", new_period);
        return -1;
    }
    period = new_period;
    char path[MAX];
    snprintf(path, MAX, "%s%s", pins[pin].path, "/period");

    char to_write[MAX];
    snprintf(to_write, MAX, "%lld", period);
    
    
    if (Open_and_write(path, to_write) < 0)
    {
        return -1;
    }
    return 0;
}

int LED_Emitter_cleanup(void)
{
    assert(is_init);

    // disable the light
    char path[MAX];
    snprintf(path, MAX, "%s%s", pins[GPIO12].path, "/enable");

    char to_write[MAX];
    snprintf(to_write, MAX, "0");

    if (Open_and_write(path, to_write) < 0)
    {
        return -1;
    }

    is_init = false;
    return 0;
}

int LED_Emitter_setTimesPerSecond(int times_per_second, GPIOPINS pin)
{   
    //easy
    if (times_per_second > 2){
        long long new_period = NANO_SECOND_TO_SECOND / times_per_second;
        if (LED_Emitter_setPeriod(new_period, pin) < 0)
        {
        return -1;
        }
        
        if (LED_Emitter_setDutyCycle(duty_cycle, pin) < 0)
        {
        return -1;
        }
    }
    // nothing
    else if (times_per_second == 0){

        char path[MAX];
        snprintf(path, MAX, "%s%s", pins[pin].path, "/duty_cycle");
        char to_write[MAX];
        snprintf(to_write, MAX, "0");
        
        if (Open_and_write(path, to_write) < 0)
        {
            return -1;
        }
    }
    // sofware solution
    else if (times_per_second == 1){

    }
    // just getting close but not perfect 0.469 seconds period 
    else if (times_per_second == 2){
        
        long long new_period = 469754879;
        if (LED_Emitter_setPeriod(new_period, pin) < 0)
        {
        return -1;
        }
        
        if (LED_Emitter_setDutyCycle(duty_cycle, pin) < 0)
        {
        return -1;
        }
    }
    else{
        printf("How did you get here?? something is not right in LED_Emitter_setTimesPerSecond\n");
    }
    return 0;
}

static int Open_and_write(char* path, char* to_write)
{
    FILE *file = fopen(path, "w+");
    if (file == NULL) {
        perror("Error opening file\n");
        return -1;
    }

    // set the initial period
    if (fprintf(file, to_write) <= 0) {
        perror("Error writing data\n");
        return -1;
    }
    fclose(file);
    return 0;
}