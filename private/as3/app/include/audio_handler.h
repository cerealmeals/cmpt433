#ifndef AUDIO_HANDLER_H
#define AUDIO_HANDLER_H
#include <stdbool.h>


typedef enum {
    BEAT_NONE = 0,
    BEAT_ROCK,
    BEAT_CUSTOM,
    NUMBER_OF_MODES
} DrumBeatMode;

typedef enum {
    SOUND_BASS_DRUM = 0,
    SOUND_HI_HAT,
    SOUND_SNARE,
    SOUND_CYMBAL,
    NUMBER_OF_SOUNDS
} DrumSound;




void audio_handler_init();
void audio_handler_cleanup();
int audio_handler_getBeatMode();
void audio_handler_startBeat(DrumBeatMode mode);
void audio_handler_cycleBeatMode();
int audio_handler_getBPM();
void audio_handler_setBPM(int new_bpm);
void audio_handler_adjustBPM(bool increase);
int audio_handler_getVolume();
void audio_handler_setVolume(int new_volume);
void audio_handler_adjustVolume(bool increase);
void audio_handler_playSound(DrumSound sound);


#endif // AUDIO_HANDLER_H
