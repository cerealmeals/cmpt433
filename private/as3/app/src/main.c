#include "shutdown.h"
#include <stdio.h>
#include "Watch_gpio.h"
#include "Rotary_Encoder_statemachine.h"
#include "Button_statemachine.h"
#include "audioMixer.h"

#define PATH_TO_SPLASH_HARD "wave-files/100060__menegass__gui-drum-splash-hard.wav"
#define PATH_TO_BT_HARD     "wave-files/100051__menegass__gui-drum-bd-hard.wav"

static int last_count = 0;
static int volume = 10;
static int max = 50;
static int min = 0;

void button(int counter)
{
    printf("counter: %d\n", counter);
    printf("button has been clicked\n");
    if(counter == 3){
        //shutdown_shutdown();
    }
}

void Rotation_handleChange(int counter)
{
    if(last_count > counter && volume < max){
        volume++;
        printf("New volume: %d \n", volume);
    }
    else if(last_count < counter && volume > min){
        volume--;
        printf("New volume: %d \n", volume);
    }
    last_count = counter;
}

static void MyNanoSleep(double mSeconds)
{
    struct timespec reqDelay;
    reqDelay.tv_sec = (time_t)(mSeconds / 1000.0);  // Convert ms to sec
    reqDelay.tv_nsec = (long)((mSeconds - (reqDelay.tv_sec * 1000.0)) * 1e6);
    nanosleep(&reqDelay, (struct timespec *) NULL);
}

int main(){

    // initialize everything
    shutdown_init();
    
    Rotary_Encoder_statemachine_init(Rotation_handleChange);
    Button_statemachine_init(button);
    Watch_gpio_Start_Watching();
    AudioMixer_init();
    
    wavedata_t symbols;
    AudioMixer_readWaveFileIntoMemory(PATH_TO_SPLASH_HARD, &symbols);
    wavedata_t drum;
    AudioMixer_readWaveFileIntoMemory(PATH_TO_BT_HARD, &drum);

    for(int i = 0; i < 6; i++){
        //printf("play sound\n");
        AudioMixer_queueSound(&symbols);
        AudioMixer_queueSound(&drum);
        
        MyNanoSleep(1000.0);
    }
    //printf("Main initialization done and is now waiting for shutdown\n");
    // wait for something to call the shutdown
    //shutdown_waitForShutdown();
    AudioMixer_freeWaveFileData(&drum);
    AudioMixer_freeWaveFileData(&symbols);
    AudioMixer_cleanup();
    printf("Main recevied shutdown\nCleaning up\n");
    Rotary_Encoder_statemachine_cleanup();
    Button_statemachine_cleanup();
    Watch_gpio_cleanup();
    shutdown_cleanup();

    printf("Main finished cleaning up\n");

    return 0;
}