#include "Watch_gpio.h"

#include <stdlib.h>
#include <stdio.h>

#include <pthread.h>
#include <assert.h>
#include <signal.h>

#define INITIAL_GPIO_CAPACITY 2  // Initial dynamic array capacity



// Structure to hold GPIO information and its callback function
typedef struct {
    enum eGpioChips chip_number;
    int line_number;
    gpio_callback_t callback;
} gpio_entry_t;

// Dynamic array and metadata
static gpio_entry_t *gpio_list = NULL;
static struct GpioLine **gpio_lines = NULL;  // Pointer to the array of GpioLine* handles
static int gpio_count = 0;       // Number of registered GPIOs
static int gpio_capacity = INITIAL_GPIO_CAPACITY;    // Current allocated capacity

static bool is_init = false;
static sig_atomic_t thread_started = false;
// Mutex for thread safety
pthread_mutex_t gpio_list_mutex = PTHREAD_MUTEX_INITIALIZER;


static volatile sig_atomic_t shutdown_requested = 0;
static pthread_t Watch_gpio_thread;
static void * thread_function(void* arg);




void Watch_gpio_cleanup()
{
    assert(is_init);
    if (pthread_kill(Watch_gpio_thread, SIGUSR1) != 0) {
        perror("pthread_kill failed");
        exit(EXIT_FAILURE);
    }
    //printf("State machine shutdown requested\n");
    
    if (pthread_join(Watch_gpio_thread, NULL) != 0) {
        perror("pthread_join failed");
        exit(EXIT_FAILURE);
    }

    free(gpio_list);
    gpio_list = NULL;

    for (int i = 0; i < gpio_count; i++){
        Gpio_close(gpio_lines[i]);
    }

    free(gpio_lines);
    gpio_lines = NULL;
    gpio_count = 0;
    gpio_capacity = 0;

    Gpio_cleanup();
}


int Watch_gpio_Add_GPIO_Line(enum eGpioChips chip, int line, gpio_callback_t callback) {
    if(!is_init){
        Gpio_initialize();
        is_init = true;
    }
    pthread_mutex_lock(&gpio_list_mutex);

    assert(!thread_started);

    // Check if the GPIO is already registered
    for (int i = 0; i < gpio_count; i++) {
        if (gpio_list[i].chip_number == chip && gpio_list[i].line_number == line) {
            pthread_mutex_unlock(&gpio_list_mutex);
            return -1;
        }
    }

    // Resize the gpio_list and gpio_lines
    //if (gpio_count >= gpio_capacity) {
        gpio_entry_t *new_list = realloc(gpio_list, (gpio_count+1) * sizeof(gpio_entry_t));
        if (!new_list) {
            perror("Failed to allocate memory for gpio_list");
            pthread_mutex_unlock(&gpio_list_mutex);
            return -2;
        }

        gpio_list = new_list;

        struct GpioLine **new_lines = realloc(gpio_lines, (gpio_count+1) * sizeof(struct GpioLine*));
        if (!new_lines) {
            perror("Failed to allocate memory for gpio_lines");
            pthread_mutex_unlock(&gpio_list_mutex);
            return -2;
        }

        gpio_lines = new_lines;
    //}

    // Open the GPIO line and add it to the gpio_lines array
    struct GpioLine* new_line_handle = Gpio_openForEvents(chip, line);
    if (!new_line_handle) {
        perror("Failed to open GPIO line");
        pthread_mutex_unlock(&gpio_list_mutex);
        return-3;
    }


    // Add new GPIO entry and store the line handle
    printf("gpio_count: %d\n", gpio_count);
    gpio_list[gpio_count].chip_number = chip;
    gpio_list[gpio_count].line_number = line;
    gpio_list[gpio_count].callback = callback;

    gpio_lines[gpio_count] = new_line_handle;
    
    gpio_count++;

    pthread_mutex_unlock(&gpio_list_mutex);
    return 1;
}

void Watch_gpio_Start_Watching()
{    
    assert(is_init);
    assert(gpio_count > 0);
    pthread_mutex_lock(&gpio_list_mutex);

    // Create thread and pass the callback
    if (pthread_create(&Watch_gpio_thread, NULL, thread_function, NULL) != 0) {
        perror("Failed to create thread\n");
        pthread_mutex_unlock(&gpio_list_mutex);
        return;
    }
    thread_started = true;

    pthread_mutex_unlock(&gpio_list_mutex);

}

static void State_signal_handler(int sig) {
    if (sig == SIGUSR1) {
        //printf("Received SIGUSR1 in Output, shutting down...\n");
        shutdown_requested = 1;
    }
}

static void *thread_function(void *arg) {

    if (arg != NULL){
        return NULL;
    }
    signal(SIGUSR1, State_signal_handler);

    while (!shutdown_requested) {
        struct gpiod_line_bulk bulkEvents;
        for (int i = 0; i < gpio_count; i++) {
            if (gpio_lines[i] == NULL) {
                printf("Error: GPIO line %d is NULL!\n", i);
            }
        }
        int numEvents = Gpio_waitForLineChange(gpio_lines, gpio_count, &bulkEvents);

        for (int i = 0; i < numEvents; i++) {
            struct gpiod_line *line_handle = gpiod_line_bulk_get_line(&bulkEvents, i);
            unsigned int this_line_number = gpiod_line_offset(line_handle);

            struct gpiod_line_event event;
            if (gpiod_line_event_read(line_handle, &event) == -1) {
                perror("Line Event");
                exit(EXIT_FAILURE);
            }

            bool isRising = (event.event_type == GPIOD_LINE_EVENT_RISING_EDGE);

            // Lookup the callback for this line and execute it
            
            for (int j = 0; j < gpio_count; j++) {
                if ((unsigned int)gpio_list[j].line_number == this_line_number && gpio_list[j].callback) {
                    gpio_callback_t callback = gpio_list[j].callback;
                    
                    callback(isRising);  // Execute callback with isRising flag
                    break;
                }
            }
            
        }
    }

    return NULL;
}