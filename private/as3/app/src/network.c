// network.c
#include "network.h"
#include "UDP.h"
#include "audio_handler.h"
#include "shutdown.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#define NUM_COMMANDS (sizeof(commands) / sizeof(commands[0]))

static void callback_function(char* message, char* response, int size_of_response);
static void mode_function(char* message, char* response, int size_of_response);
static void volume_function(char* message, char* response, int size_of_response);
static void tempo_function(char* message, char* response, int size_of_response);
static void play_sound_function(char* message, char* response, int size_of_response);
static void stop_function(char* message, char* response, int size_of_response);

typedef struct {
    char* string;
    network_callback_t callback;
} List_Of_Commands;

List_Of_Commands commands[] = {
    {"mode", mode_function},
    {"volume", volume_function},
    {"tempo", tempo_function},
    {"play", play_sound_function},
    {"stop", stop_function}
};

static bool is_init = false;

void network_init(void)
{
    assert(!is_init);
    UDP_init(callback_function);
    is_init = true;
}

void network_cleanup(void)
{
    assert(is_init);
    UDP_cleanup();
    is_init = false;
}

static void callback_function(char* message, char* response, int size_of_response)
{
    response[0] = '\0'; // Default empty response
    
    char *cmd = strtok(message, " "); // Get command
    char *arg = strtok(NULL, " ");    // Get argument (if any)
    
    if (cmd == NULL) {
        snprintf(response, size_of_response, "ERROR: Empty command");
        return;
    }
    
    for (size_t i = 0; i < NUM_COMMANDS; i++) {
        if (strcmp(cmd, commands[i].string) == 0) {
            commands[i].callback(arg, response, size_of_response);
            return;
        }
    }
    
    snprintf(response, size_of_response, "ERROR: Unknown command");
}

static void mode_function(char* message, char* response, int size_of_response)
{
    if (!message) {
        snprintf(response, size_of_response, "ERROR: No mode specified");
        return;
    }
    int mode = atoi(message);
    audio_handler_startBeat(mode);
    snprintf(response, size_of_response, "Mode set to %d", mode);
}

static void volume_function(char* message, char* response, int size_of_response)
{
    if (!message) {
        snprintf(response, size_of_response, "ERROR: No volume specified");
        return;
    }
    int volume = atoi(message);
    audio_handler_setVolume(volume);
    snprintf(response, size_of_response, "Volume set to %d", volume);
}

static void tempo_function(char* message, char* response, int size_of_response)
{
    if (!message) {
        snprintf(response, size_of_response, "ERROR: No tempo specified");
        return;
    }
    int tempo = atoi(message);
    audio_handler_setBPM(tempo);
    snprintf(response, size_of_response, "Tempo set to %d BPM", tempo);
}

static void play_sound_function(char* message, char* response, int size_of_response)
{
    if (!message) {
        snprintf(response, size_of_response, "ERROR: No sound ID specified");
        return;
    }
    int sound_id = atoi(message);
    audio_handler_playSound(sound_id);
    snprintf(response, size_of_response, "Playing sound %d", sound_id);
}

static void stop_function(char* message, char* response, int size_of_response)
{
    if (message) {
        snprintf(response, size_of_response, "ERROR: Stop command had extra data");
        return;
    }
    shutdown_shutdown();
    snprintf(response, size_of_response, "Playback stopped");
}