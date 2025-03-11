#include "Watch_gpio.h"

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <signal.h>
#include <stdbool.h>

#define INITIAL_GPIO_CAPACITY 2  

// Structure to hold GPIO information and its callback function
typedef struct {
    int line_number;
    gpio_callback_t callback;
} gpio_entry_t;

// Per-chip data storage
typedef struct {
    enum eGpioChips chip;
    gpio_entry_t *gpio_list;
    struct GpioLine **gpio_lines;
    int gpio_count;
    int gpio_capacity;
    pthread_t thread;
} gpio_chip_data_t;

// Dynamic array of chips being monitored
static gpio_chip_data_t *gpio_chips = NULL;
static int chip_count = 0;
static int chip_capacity = 0;
static bool is_init = false;
static sig_atomic_t shutdown_requested = 0;
pthread_mutex_t gpio_list_mutex = PTHREAD_MUTEX_INITIALIZER;

// Function declarations
static void *thread_function(void* arg);
static void State_signal_handler(int sig);

// Cleanup function
void Watch_gpio_cleanup() {
    assert(is_init);
    
    for (int i = 0; i < chip_count; i++) {
        pthread_kill(gpio_chips[i].thread, SIGUSR1);
        pthread_join(gpio_chips[i].thread, NULL);
        
        for (int j = 0; j < gpio_chips[i].gpio_count; j++) {
            Gpio_close(gpio_chips[i].gpio_lines[j]);
        }

        free(gpio_chips[i].gpio_list);
        free(gpio_chips[i].gpio_lines);
    }

    free(gpio_chips);
    gpio_chips = NULL;
    chip_count = 0;
    chip_capacity = 0;

    Gpio_cleanup();
}

// Find or create a chip entry
static gpio_chip_data_t *get_chip_data(enum eGpioChips chip) {
    for (int i = 0; i < chip_count; i++) {
        if (gpio_chips[i].chip == chip) return &gpio_chips[i];
    }

    // Expand chip list if needed
    if (chip_count >= chip_capacity) {
        chip_capacity += 2;
        gpio_chip_data_t *new_chips = realloc(gpio_chips, chip_capacity * sizeof(gpio_chip_data_t));
        if (!new_chips) {
            perror("Failed to allocate memory for gpio_chips");
            return NULL;
        }
        gpio_chips = new_chips;
    }

    // Add new chip entry
    gpio_chip_data_t *chip_data = &gpio_chips[chip_count++];
    chip_data->chip = chip;
    chip_data->gpio_list = malloc(INITIAL_GPIO_CAPACITY * sizeof(gpio_entry_t));
    chip_data->gpio_lines = malloc(INITIAL_GPIO_CAPACITY * sizeof(struct GpioLine *));
    chip_data->gpio_count = 0;
    chip_data->gpio_capacity = INITIAL_GPIO_CAPACITY;

    return chip_data;
}

// Add a GPIO line
int Watch_gpio_Add_GPIO_Line(enum eGpioChips chip, int line, gpio_callback_t callback) {
    if (!is_init) {
        Gpio_initialize();
        is_init = true;
    }

    pthread_mutex_lock(&gpio_list_mutex);

    gpio_chip_data_t *chip_data = get_chip_data(chip);
    if (!chip_data) {
        pthread_mutex_unlock(&gpio_list_mutex);
        return -1;
    }

    // Check if line is already registered
    for (int i = 0; i < chip_data->gpio_count; i++) {
        if (chip_data->gpio_list[i].line_number == line) {
            pthread_mutex_unlock(&gpio_list_mutex);
            return -2;
        }
    }

    // Expand if necessary
    if (chip_data->gpio_count >= chip_data->gpio_capacity) {
        chip_data->gpio_capacity *= 2;
        chip_data->gpio_list = realloc(chip_data->gpio_list, chip_data->gpio_capacity * sizeof(gpio_entry_t));
        chip_data->gpio_lines = realloc(chip_data->gpio_lines, chip_data->gpio_capacity * sizeof(struct GpioLine *));
        if (!chip_data->gpio_list || !chip_data->gpio_lines) {
            perror("Failed to allocate memory");
            pthread_mutex_unlock(&gpio_list_mutex);
            return -3;
        }
    }

    // Open GPIO line
    struct GpioLine* new_line_handle = Gpio_openForEvents(chip, line);
    if (!new_line_handle) {
        perror("Failed to open GPIO line");
        pthread_mutex_unlock(&gpio_list_mutex);
        return -4;
    }

    // Store GPIO entry
    int index = chip_data->gpio_count++;
    chip_data->gpio_list[index].line_number = line;
    chip_data->gpio_list[index].callback = callback;
    chip_data->gpio_lines[index] = new_line_handle;

    pthread_mutex_unlock(&gpio_list_mutex);
    return 1;
}

// Start monitoring GPIOs
void Watch_gpio_Start_Watching() {
    assert(is_init);
    assert(chip_count > 0);

    pthread_mutex_lock(&gpio_list_mutex);

    for (int i = 0; i < chip_count; i++) {
        if (pthread_create(&gpio_chips[i].thread, NULL, thread_function, &gpio_chips[i]) != 0) {
            perror("Failed to create thread");
            pthread_mutex_unlock(&gpio_list_mutex);
            return;
        }
    }

    pthread_mutex_unlock(&gpio_list_mutex);
}

// Signal handler for cleanup
static void State_signal_handler(int sig) {
    if (sig == SIGUSR1) {
        shutdown_requested = 1;
    }
}

// GPIO monitoring thread (one per chip)
static void *thread_function(void *arg) {
    gpio_chip_data_t *chip_data = (gpio_chip_data_t *)arg;
    signal(SIGUSR1, State_signal_handler);

    while (!shutdown_requested) {
        struct gpiod_line_bulk bulkEvents;
        int numEvents = Gpio_waitForLineChange(chip_data->gpio_lines, chip_data->gpio_count, &bulkEvents);

        for (int i = 0; i < numEvents; i++) {
            struct gpiod_line *line_handle = gpiod_line_bulk_get_line(&bulkEvents, i);
            unsigned int this_line_number = gpiod_line_offset(line_handle);

            // Find the matching GPIO entry
            for (int j = 0; j < chip_data->gpio_count; j++) {
                if ((unsigned int)chip_data->gpio_list[j].line_number == this_line_number) {
                    struct gpiod_line_event event;
                    if (gpiod_line_event_read(line_handle, &event) == -1) {
                        perror("Line Event");
                        exit(EXIT_FAILURE);
                    }
                    bool isRising = event.event_type == GPIOD_LINE_EVENT_RISING_EDGE;
                    chip_data->gpio_list[j].callback(isRising);
                    break;
                }
            }
        }
    }

    return NULL;
}
