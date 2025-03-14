#include "audio_handler.h"
#include "audioMixer.h"
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <assert.h>


#define PATH_TO_BT_HARD     "wave-files/100051__menegass__gui-drum-bd-hard.wav"         // base drum
#define PATH_TO_CC          "wave-files/100053__menegass__gui-drum-cc.wav"              // hi-hat
#define PATH_TO_SNARE_SOFT  "wave-files/100059__menegass__gui-drum-snare-soft.wav"      // snare
#define PATH_TO_SPLASH_HARD "wave-files/100060__menegass__gui-drum-splash-hard.wav"



static wavedata_t sound_data[NUMBER_OF_SOUNDS];


static pthread_mutex_t beat_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t drum_thread;
static bool playing = true;
static void* drum_beat_loop(void* arg);

#define MAX_BEATS_PER_LINE 2
#define MAX_LINES 4

typedef struct {
    DrumSound beats[MAX_LINES][MAX_BEATS_PER_LINE];
    int number_of_lines;
    int number_of_max_sounds_per_line;
} DrumStep;

static DrumStep beat_modes[NUMBER_OF_MODES] = {
    // BEAT_NONE (No sounds)
    {{{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}}, MAX_LINES, MAX_BEATS_PER_LINE},

    // BEAT_ROCK
    {{{SOUND_HI_HAT, SOUND_BASS_DRUM},
      {SOUND_HI_HAT, -1},
      {SOUND_HI_HAT, SOUND_SNARE},
      {SOUND_HI_HAT, -1}}, MAX_LINES, MAX_BEATS_PER_LINE},

    // BEAT_CUSTOM
    {{{SOUND_HI_HAT, SOUND_BASS_DRUM},
      {SOUND_HI_HAT, -1},
      {SOUND_HI_HAT, SOUND_CYMBAL},
      {SOUND_HI_HAT, -1}}, MAX_LINES, MAX_BEATS_PER_LINE},
};
static DrumBeatMode beat_mode_index = BEAT_NONE;

static int bpm = 120;  
static int volume = 80; 

static bool is_init = false;

static void MyNanoSleep(double mSeconds)
{
    struct timespec reqDelay;
    reqDelay.tv_sec = (time_t)(mSeconds / 1000.0);
    reqDelay.tv_nsec = (long)((mSeconds - (reqDelay.tv_sec * 1000.0)) * 1e6);
    nanosleep(&reqDelay, NULL);
}

void audio_handler_init()
{
    assert(!is_init);
    is_init = true;
    char* paths[NUMBER_OF_SOUNDS] = {
        PATH_TO_BT_HARD,    
        PATH_TO_CC,         
        PATH_TO_SNARE_SOFT, 
        PATH_TO_SPLASH_HARD
    };

    AudioMixer_init();
    for (int i = 0; i < NUMBER_OF_SOUNDS; i++) {
        AudioMixer_readWaveFileIntoMemory(paths[i], &sound_data[i]);
    }
    AudioMixer_setVolume(volume);

    pthread_create(&drum_thread, NULL, drum_beat_loop, NULL);
}

void audio_handler_startBeat(DrumBeatMode mode)
{
    if (mode < BEAT_NONE || mode > BEAT_CUSTOM){
        printf("Not a Valid Beat\n");
        return;
    }
    pthread_mutex_lock(&beat_mutex);
    beat_mode_index = mode;
    pthread_mutex_unlock(&beat_mutex);
}

void audio_handler_cycleBeatMode()
{
    pthread_mutex_lock(&beat_mutex);
    beat_mode_index = (beat_mode_index + 1) % NUMBER_OF_MODES;
    pthread_mutex_unlock(&beat_mutex);
    printf("Beat mode %d\n", beat_mode_index);
}

void audio_handler_setBPM(int new_bpm)
{
    if (new_bpm < 40) bpm = 40;
    else if (new_bpm > 300) bpm = 300;
    else bpm = new_bpm;
}

void audio_handler_adjustBPM(bool increase)
{
    if (increase) {
        bpm = (bpm + 5 > 300) ? 300 : bpm + 5;
    } else {
        bpm = (bpm - 5 < 40) ? 40 : bpm - 5;
    }
}

void audio_handler_setVolume(int new_volume)
{
    if (new_volume < 0) volume = 0;
    else if (new_volume > 100) volume = 100;
    else volume = new_volume;

    AudioMixer_setVolume(volume);
}

void audio_handler_adjustVolume(bool increase)
{
    if (increase) {
        volume = (volume + 5 > 100) ? 100 : volume + 5;
    } else {
        volume = (volume - 5 < 0) ? 0 : volume - 5;
    }
    AudioMixer_setVolume(volume);
}

void audio_handler_playSound(DrumSound sound)
{
    if (sound >= 0 && sound < NUMBER_OF_SOUNDS) {
        AudioMixer_queueSound(&sound_data[sound]);
    }
}

void audio_handler_cleanup()
{
    assert(is_init);
    
    playing = false;
    pthread_join(drum_thread, NULL);

    for (int i = 0; i < NUMBER_OF_SOUNDS; i++) {
        AudioMixer_freeWaveFileData(&sound_data[i]);
    }

    AudioMixer_cleanup();
    is_init = false;
}

static void* drum_beat_loop(void* arg)
{
    if (arg != NULL){
        printf("drum_beat_loop should not have args");
    }
    while (playing)
    {
        pthread_mutex_lock(&beat_mutex);
        DrumBeatMode mode = beat_mode_index;
        pthread_mutex_unlock(&beat_mutex);

        if (mode == BEAT_NONE) {
            MyNanoSleep(100.0);
            continue;
        }

        DrumStep *pattern = &beat_modes[mode];
        for (int i = 0; i < pattern->number_of_lines; i++)
        {
            for (int j = 0; j < pattern->number_of_max_sounds_per_line; j++) {
                if ((int)pattern->beats[i][j] != -1) {
                    AudioMixer_queueSound(&sound_data[pattern->beats[i][j]]);
                }
            }
            MyNanoSleep(60000.0 / bpm);
        }
    }
    return NULL;
}
