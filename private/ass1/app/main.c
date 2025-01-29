#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "/home/debian/cmpt433/private/hal/LEDcontrol.h"
#include <time.h>

static void MyNanoSleep(double time);

int main(){


    char *username = getlogin();
    if (username == NULL){
        perror("getlogin");
        exit(EXIT_FAILURE);
    }

    printf("Hello embedded world, from %s\n", username);

    LEDcontrol_init();
    
    LEDcontrol_on(GREEN);
    MyNanoSleep(1.5);
    LEDcontrol_off(GREEN);
    LEDcontrol_on(RED);
    MyNanoSleep(1.5);
    LEDcontrol_off(RED);

    

    LEDcontrol_heartbeat(GREEN, false);
    MyNanoSleep(2.5);
    LEDcontrol_heartbeat(RED, true);
    MyNanoSleep(2.5);
    LEDcontrol_off(RED);
    LEDcontrol_off(GREEN);

    LEDcontrol_timer(RED, 100, 200);
    MyNanoSleep(2.5);
    LEDcontrol_off(RED);
    LEDcontrol_timer(GREEN, 200, 100);
    MyNanoSleep(2.5);
    LEDcontrol_off(GREEN);

    printf("both should be off\n");
    exit(EXIT_SUCCESS);
}

static void MyNanoSleep(double time)
{
    // Sleep 1.5 seconds
    long seconds = (long)time;
    double fraction = time - seconds;
    long nanoseconds = (long)(fraction * 100000000);
    struct timespec reqDelay = {seconds, nanoseconds};
    nanosleep(&reqDelay, (struct timespec *) NULL);
}