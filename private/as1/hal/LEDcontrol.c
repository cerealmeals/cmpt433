#include "LEDcontrol.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#define MAX_LEDS 2
#define MAX_PATH 20
//#define DEBUG

static void MyNanoSleep(double time);
static void SetDelay(LED_COLOUR colour, int delay, char* EndPath);
static void TurnOnOrOff(LED_COLOUR colour, int status);
static void SetTrigger(LED_COLOUR colour, char* string);

typedef struct {
    char path[MAX_PATH];
} LED;

static LED LEDS[MAX_LEDS];

static bool init = false;

void LEDcontrol_init(void)
{
    LED red = {"/sys/class/leds/PWR"};
    LED green = {"/sys/class/leds/ACT"};

    LEDS[0] = red;
    LEDS[1] = green;

    init = true;
}

void LEDcontrol_on(LED_COLOUR colour)
{
    assert(init == true);
    SetTrigger(colour, "none");
    TurnOnOrOff(colour, 1);
}

void LEDcontrol_off(LED_COLOUR colour)
{
    assert(init == true);
    SetTrigger(colour, "none");
    TurnOnOrOff(colour, 0);
}

void LEDcontrol_timer(LED_COLOUR colour, int on_time, int off_time)
{
    assert(init == true);

    SetTrigger(colour, "timer");
    MyNanoSleep(0.1);

    SetDelay(colour, on_time, "/delay_on");
    SetDelay(colour, off_time, "/delay_off");
}

void LEDcontrol_heartbeat(LED_COLOUR colour, bool invert)
{
    assert(init == true);
    SetTrigger(colour, "heartbeat");
    MyNanoSleep(0.1);


    const int max = 27;
    char path[max];
    snprintf(path, max, "%s%s", LEDS[colour].path, "/invert");

#ifdef DEBUG
    printf("hearthbeat check the file route: %s\n", path);
#endif
    FILE *DelayFile = fopen(path, "w+");
    if (DelayFile == NULL) {
        perror("Error opening LED heartbeat file");
        exit(EXIT_FAILURE);
    }

    int charWritten;
    if (invert == true){
        charWritten = fprintf(DelayFile, "1");
    }
    else{
        charWritten = fprintf(DelayFile, "0");
    }
    
    if (charWritten <= 0) {
        perror("Error writing data to LED heartbeat file");
        exit(EXIT_FAILURE); 
    }
    fclose(DelayFile);
}

// char* LEDcontrol_trigger(LED_COLOUR colour)
// {
//     assert(init == true);

// }

// bool LEDcontrol_status(LED_COLOUR colour)
// {
//     assert(init == true);

// }

static void MyNanoSleep(double time)
{
    // Sleep 1.5 seconds
    long seconds = (long)time;
    double fraction = time - seconds;
    long nanoseconds = (long)(fraction * 100000000);
    struct timespec reqDelay = {seconds, nanoseconds};
    nanosleep(&reqDelay, (struct timespec *) NULL);
}

static void SetDelay(LED_COLOUR colour, int delay, char* EndPath)
{
    const int max = 30;
    char path[max];
    snprintf(path, max, "%s%s", LEDS[colour].path, EndPath);

#ifdef DEBUG
    printf("SetDelay check the file route: %s\n", path);
#endif
    FILE *DelayFile = fopen(path, "w+");
    if (DelayFile == NULL) {
        perror("Error opening LED delay file");
        exit(EXIT_FAILURE);
    }

    const int CapForDelayLen = 10;
    char str[CapForDelayLen];
    snprintf(str, CapForDelayLen, "%d", delay);

    int charWritten = fprintf(DelayFile, str);
    if (charWritten <= 0) {
        perror("Error writing data to LED delay file");
        exit(EXIT_FAILURE); 
    }
    fclose(DelayFile);
}

static void TurnOnOrOff(LED_COLOUR colour, int status)
{
    const int max = 31;
    char path[max];
    snprintf(path, max, "%s%s", LEDS[colour].path, "/brightness");

    #ifdef DEBUG
    printf("TurnOnOrOff check the file route: %s\n", path);
    #endif
    FILE *BrightnessFile = fopen(path, "w+");
    if (BrightnessFile == NULL) {
        perror("Error opening LED Brightness file");
        exit(EXIT_FAILURE);
    }

    const int forCappingFileInput = 2;
    char ch[forCappingFileInput];
    snprintf(ch, forCappingFileInput, "%d", status);

    int charWritten = fprintf(BrightnessFile, ch);
    if (charWritten <= 0) {
        perror("Error writing data to LED brightness file");
        exit(EXIT_FAILURE); 
    }
    fclose(BrightnessFile);
}

static void SetTrigger(LED_COLOUR colour, char* string)
{
    const int max = 28;
    char path[max];
    snprintf(path, max, "%s%s", LEDS[colour].path, "/trigger");

    #ifdef DEBUG
    printf("SetTrigger check the file route: %s\n", path);
    #endif
    FILE *TriggerFile = fopen(path, "w+");
    if (TriggerFile == NULL) {
        perror("Error opening LED trigger file");
        exit(EXIT_FAILURE);
    }  

    int charWritten = fprintf(TriggerFile, string);
    if (charWritten <= 0) {
        perror("Error writing data to LED trigger file");
        exit(EXIT_FAILURE); 
    }
    fclose(TriggerFile);
}
