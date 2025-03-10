#include "gpio.h"
#include <stdlib.h>
#include <stdio.h>
#include <gpiod.h>
#include <assert.h>

// Relies on the gpiod library.
// Insallation for cross compiling:
//      (host)$ sudo dpkg --add-architecture arm64
//      (host)$ sudo apt update
//      (host)$ sudo apt install libgpdiod-dev:arm64
// GPIO: https://www.ics.com/blog/gpio-programming-exploring-libgpiod-library
// Example: https://github.com/starnight/libgpiod-example/blob/master/libgpiod-input/main.c

// TYPE NOTE:
// Internally cast the 
//    struct GpioLine* 
// to 
//    (struct gpiod_line*)
// so we hide the dependency on gpiod


static bool s_isInitialized = false;

static char* s_chipNames[] = {
    "gpiochip0",
    "gpiochip1",
    "gpiochip2",
};

// Hold open chips
static struct gpiod_chip* s_openGpiodChips[GPIO_NUM_CHIPS];

void Gpio_initialize(void)
{
    assert(!s_isInitialized);
    for (int i = 0; i < GPIO_NUM_CHIPS; i++) {
         // Open GPIO chip
        s_openGpiodChips[i] = gpiod_chip_open_by_name(s_chipNames[i]);
        if (!s_openGpiodChips[i]) {
            perror("GPIO Initializing: Unable to open GPIO chip");
            exit(EXIT_FAILURE);
        }
    }
    s_isInitialized = true;
}

void Gpio_cleanup(void)
{
    assert(s_isInitialized);
    for (int i = 0; i < GPIO_NUM_CHIPS; i++) {
        // Close GPIO chip
        gpiod_chip_close(s_openGpiodChips[i]);
        if (!s_openGpiodChips[i]) {
            perror("GPIO Initializing: Unable to open GPIO chip");
            exit(EXIT_FAILURE);
        }
    }
    s_isInitialized = false;
}

// Opening a pin gives us a "line" that we later work with.
//  chip: such as GPIO_CHIP_0
//  pinNumber: such as 15
struct GpioLine* Gpio_openForEvents(enum eGpioChips chip, int pinNumber)
{
    assert(s_isInitialized);
    struct gpiod_chip* gpiodChip = s_openGpiodChips[chip];
    struct gpiod_line* line = gpiod_chip_get_line(gpiodChip, pinNumber);
    if (!line) {
        perror("Unable to get GPIO line");
        exit(EXIT_FAILURE);
    }

    int direction = gpiod_line_direction(line);
    printf("Pin %d direction: %s\n", pinNumber, direction == GPIOD_LINE_DIRECTION_INPUT ? "INPUT" : "OUTPUT");

    return (struct GpioLine*) line;  
}

void Gpio_close(struct GpioLine* line)
{
    assert(s_isInitialized);
    gpiod_line_release((struct gpiod_line*) line);
}


// Returns the number of events
int Gpio_waitForLineChange(
    struct GpioLine** lines, 
    int numLines,
    struct gpiod_line_bulk *bulkEvents
) {
    assert(s_isInitialized);

    // Source: https://people.eng.unimelb.edu.au/pbeuchat/asclinic/software/building_block_gpio_encoder_counting.html   
    struct gpiod_line_bulk bulkWait;
    gpiod_line_bulk_init(&bulkWait);
    
    // Add all lines to the bulk structure
    for (int i = 0; i < numLines; i++) {
        struct gpiod_line* next_line_to_add = (struct gpiod_line*)lines[i];
        gpiod_line_bulk_add(&bulkWait, next_line_to_add);
    }

    for (int i = 0; i < numLines; i++) {
        if (bulkWait.lines[i] == NULL) {
            fprintf(stderr, "Error: GPIO line at index %d is NULL\n", i);
            return -1;
        }
    }
    if (gpiod_line_request_bulk_both_edges_events(&bulkWait, "Event Waiting") < 0) {
        perror("Error on Request gpio.c line 113:");
        exit(-1);
    }


    int result = gpiod_line_event_wait_bulk(&bulkWait, NULL, bulkEvents);
    if ( result == -1) {
        perror("Error waiting on lines for event waiting");
        return (int)-1;
    }

    int numEvents = gpiod_line_bulk_num_lines(bulkEvents);
    return numEvents;
}
