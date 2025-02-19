#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "LEDcontrol.h"
#include "JoystickControl.h"
#include "JoystickInterp.h"
#include <time.h>
//#define DEBUG

typedef enum {
    Early = 0,
    Exit,
    Waited
}WaitResult;

static void MyNanoSleep(double time);
static bool GetReady(void);
static WaitResult Wait(void);
static bool Choose(void);
static Direction JustCheckingJoystick(double time);
static long long getTimeInMs(void);
static void flash5time(LED_COLOUR);

static int besttime = 5001;


int main(){

    char *username = getlogin();
    if (username == NULL){
        perror("getlogin\n");
        exit(EXIT_FAILURE);
    }
    printf("Hello embedded world, from %s\n", username);

    LEDcontrol_init();

    // main loop
    while(true){
        bool stop = GetReady();
        if(stop){
            break;
        }
        while (JoystickInterp() == UP || JoystickInterp() == DOWN){
            printf("Please let go of the joystick\n");
            MyNanoSleep(0.5);
        }
        WaitResult check = Wait();
        if(check == Exit){
            break;
        }
        else if (check == Waited){
            stop = Choose();
            if(stop){
                break;
            }
        }
    }

    LEDcontrol_off(GREEN);
    LEDcontrol_off(RED);
    JoystickControl_close();
    printf("Thanks for playing\n");
    exit(EXIT_SUCCESS);
}

static void MyNanoSleep(double time)
{
    struct timespec reqDelay;
    reqDelay.tv_sec = (time_t)time;
    double fraction = time - (double)reqDelay.tv_sec;
    reqDelay.tv_nsec = (long)(fraction * 1e9);
    nanosleep(&reqDelay, (struct timespec *) NULL);
}

//return true if ending the game
static bool GetReady(void)
{
    printf("get ready...\n");

    const int flashes = 4;
    for(int i = 0; i < flashes; i++){
        #ifdef DEBUG
            printf("check 63\n");
        #endif
        LEDcontrol_on(GREEN);
        Direction dir = JustCheckingJoystick(0.25);
        if(dir == RIGHT || dir == LEFT){
            printf("You pressed right or left. Exiting\n");
            return true;
        }
        LEDcontrol_off(GREEN);
        LEDcontrol_on(RED);
        dir = JustCheckingJoystick(0.25);
        if(dir == RIGHT || dir == LEFT){
            printf("You pressed right or left. Exiting\n");
            return true;
        }
        LEDcontrol_off(RED);
    }
    return false;
}

static  WaitResult Wait(void)
{
    srand(time(NULL));

    // https://stackoverflow.com/questions/1202687/how-do-i-get-a-specific-range-of-numbers-from-rand
    // but I typed it myself
    int random_number = (rand() % (3500 - 500 + 1)) + 500;

    Direction dir = JustCheckingJoystick((double)random_number/1000.0);
    if(dir == RIGHT || dir == LEFT){
        printf("You pressed right or left. Exiting\n");
        return Exit;
    }
    else if (dir == UP || dir == DOWN){
        printf("Too soon\n");
        return Early;
    }
    return Waited;
}

//return true if ending the game
static bool Choose(void)
{
    long long start;
    int random_number = rand();
    Direction correctDirection;

    if (random_number > RAND_MAX/2){
        printf("UP\n");
        LEDcontrol_on(GREEN);
        correctDirection = UP;
    }
    else{
        printf("DOWN\n");
        LEDcontrol_on(RED);
        correctDirection = DOWN;
    }

    // start time and wait for player to react
    start = getTimeInMs();
    Direction dir = JustCheckingJoystick(5.0);
    int reactiontime = (int)(getTimeInMs() - start);

    LEDcontrol_off(GREEN);
    LEDcontrol_off(RED);
    
    if( dir == correctDirection){

        if (besttime > reactiontime){
            besttime = reactiontime;
            printf("You got a new best time! Congratulations\n");
        }
        printf("You reacted in %dms. Your best time in this setting so far is %dms\n", reactiontime, besttime);
        flash5time(GREEN);
    }
    else if (dir == NONE){
        printf("You didn't react within 5000ms. YOU LOSE!\n");
        return true;
    }
    else{
        printf("You didn't choose the right direction\n");
        flash5time(RED);
    }
    return false;
}

// return the direction of joystick while waiting
static Direction JustCheckingJoystick(double time)
{
    long long startTime = getTimeInMs();
    
    while (startTime + (time*1000) > getTimeInMs()){
        
        Direction dir = JoystickInterp();
        if(dir != NONE){
            return dir;
        }
        MyNanoSleep(0.01);
    }
    return NONE;
}

static long long getTimeInMs(void)
{
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    long long seconds = spec.tv_sec;
    long long nanoSeconds = spec.tv_nsec;
    long long milliSeconds = seconds * 1000
    + nanoSeconds / 1000000;
    return milliSeconds;
}

static void flash5time(LED_COLOUR colour)
{
    const int five = 5;
    for(int i = 0; i < five; i++){
        LEDcontrol_on(colour);
        MyNanoSleep(0.125);
        LEDcontrol_off(colour);
        MyNanoSleep(0.075);
    }
}
