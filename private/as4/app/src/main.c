#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "Accelerometer.h"

int main(void)
{
    printf("=== Accelerometer Test Application ===\n");

    // 1) Initialize the accelerometer (e.g., set up I2C bus, configure registers)
    Accelerometer_init();

    // 2) Interactively prompt the user
    while (1) {
        // Ask user for a command
        printf("\nType '3' to get accelerometer data, or 'exit' to quit.\n> ");

        char input[64];
        if (!fgets(input, sizeof(input), stdin)) {
            // EOF or read error => quit
            break;
        }

        // Strip trailing newline
        char *newline = strchr(input, '\n');
        if (newline) {
            *newline = '\0';
        }

        // Convert to lowercase for simpler comparison
        for (char *p = input; *p; p++) {
            *p = (char)tolower((unsigned char)*p);
        }

        if (strcmp(input, "exit") == 0) {
            // User wants to quit
            break;
        } else if (strcmp(input, "3") == 0) {
            // 3) Read the current accelerometer state
            AccelerometerOutput accelData;
            Accelerometer_GetAccelerationState(&accelData);
            printf("Accelerometer Reading:\n");
            printf("  X = %d\n", accelData.x);
            printf("  Y = %d\n", accelData.y);
            printf("  Z = %d\n", accelData.z);
        } else {
            printf("Unrecognized command: '%s'\n", input);
        }
    }

    // 4) Cleanup
    Accelerometer_cleanup();

    printf("Exiting Accelerometer Test Application.\n");
    return 0;
}
