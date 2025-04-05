#include "SharedMemory.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>   // For nanosleep or clock_gettime

#include "sharedDataLayout.h"

// Addresses from your platform’s TRM:
#define BTCM_ADDR   0x79020000
#define MEM_LENGTH  0x8000

static volatile void* base = NULL; // Pointer to mapped memory

//------------------------------------------------------------------------------
// 1) SharedMemory_init()
//    Maps the R5’s BTCM region to get a valid pointer.
//    Return 0 on success, -1 on failure.
//------------------------------------------------------------------------------
int SharedMemory_init(void)
{
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        perror("ERROR: open /dev/mem (sudo?)");
        return -1;
    }

    base = mmap(NULL, MEM_LENGTH, PROT_READ | PROT_WRITE, MAP_SHARED, fd, BTCM_ADDR);
    if (base == MAP_FAILED) {
        perror("ERROR: mmap R5 memory");
        close(fd);
        return -1;
    }
    close(fd);

    // Optionally clear the LED flag to 0, so we start fresh
    setSharedMem_uint8(base, LED_FLAG_OFFSET, 0);

    printf("SharedMemory_init(): Mapped BTCM at %p\n", base);
    return 0;
}

//------------------------------------------------------------------------------
// 2) SharedMemory_cleanup()
//    Unmaps the memory region. If you call this, no further read/writes are valid.
//------------------------------------------------------------------------------
void SharedMemory_cleanup(void)
{
    if (base != NULL) {
        if (munmap((void*)base, MEM_LENGTH)) {
            perror("ERROR: munmap R5 memory");
        }
        base = NULL;
    }
    printf("SharedMemory_cleanup(): unmapped BTCM.\n");
}

//------------------------------------------------------------------------------
// 3) Blocking call to set 8 NeoPixel colors with a timeout
//    Steps:
//      - Wait for R5 to clear LED_FLAG_OFFSET to 0, up to timeoutMs.
//      - If not cleared, return -1.
//      - If cleared, write color array to LED_0_OFFSET..LED_7_OFFSET.
//      - Set LED_FLAG_OFFSET to 1, return 0 for success.
//------------------------------------------------------------------------------
int SharedMemory_SetNeopixelColours_Blocking(const uint32_t newColors[8], int timeoutMs)
{
    if (!base) {
        fprintf(stderr, "SharedMemory_SetNeopixelColours_Blocking: Not initialized!\n");
        return -1;
    }

    // 1) Wait until R5 has cleared the flag (LED_FLAG_OFFSET == 0) or we timeout.
    const int sleepGranularityUs = 400; // 400us
    float waitedMs = 0;

    while (1) {
        uint8_t flagVal = getSharedMem_uint8(base, LED_FLAG_OFFSET);
        if (flagVal == 0) {
            // R5 is ready!
            break;
        }

        // Sleep a little
        usleep(sleepGranularityUs);
        waitedMs += 0.4;
        if (waitedMs >= timeoutMs) {
            fprintf(stderr, "Timeout waiting for R5 to clear LED_FLAG_OFFSET\n");
            return -1; // timed out
        }
    }

    // 2) Write the new 8 colors
    for (int i = 0; i < 8; i++) {
        setSharedMem_uint32(base, LED_0_OFFSET + i*sizeof(uint32_t), newColors[i]);
    }

    // 3) Set the flag to 1 => R5 sees new data is ready
    setSharedMem_uint8(base, LED_FLAG_OFFSET, 1);

    return 0; // success
}

//------------------------------------------------------------------------------
// 4) Expose the raw base pointer (optional)
//------------------------------------------------------------------------------
volatile void* SharedMemory_GetBase(void)
{
    return base;
}

#include <stdio.h>
#include "sharedDataLayout.h"

void SharedMemory_PrintSharedMemoryHex()
{
    printf("=== Debug: Printing Shared Memory (Hex) from 0x%X to 0x%lX ===\n",
        MEM_START_OFFSET, END_MEMORY_OFFSET);

    // Print 16 bytes per line
    const int bytesPerLine = 4;
    for (uint32_t offset = MEM_START_OFFSET; offset < END_MEMORY_OFFSET; offset += bytesPerLine)
    {
        // Print the line offset
        printf("0x%04X: ", offset);

        // Print each byte in hex
        for (int i = 0; i < bytesPerLine; i++) {
            uint32_t current = offset + i;
            if (current < END_MEMORY_OFFSET) {
                uint8_t val = getSharedMem_uint8(base, current);
                printf("%02X ", val);
            } else {
                // If we run past END_MEMORY_OFFSET, just space-fill
                printf("   ");
            }
        }
        printf("\n");
    }

    printf("=== End of Shared Memory Dump ===\n");
}
