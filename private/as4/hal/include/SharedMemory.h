#ifndef SHAREDMEMORY_H
#define SHAREDMEMORY_H

#include <stdint.h>


// Initializes the shared memory, mapping /dev/mem and storing the pointer.
// Returns 0 on success, -1 on error.
int SharedMemory_init(void);

// Cleans up, unmapping /dev/mem.
void SharedMemory_cleanup(void);

// Blocking function to set an array of 8 NeoPixel colors (32 bits each).
//  - Waits up to timeoutMs milliseconds for R5 to clear LED_FLAG_OFFSET to 0.
//  - If R5 doesn’t clear in time, returns -1 (timeout).
//  - If it succeeds, writes the color array and sets LED_FLAG_OFFSET to 1, then returns 0.
int SharedMemory_SetNeopixelColours_Blocking(const uint32_t newColors[8], int timeoutMs);

// (Optional) Exposes the raw pointer to shared memory for direct reads/writes
// in user code, e.g. reading button states.
volatile void* SharedMemory_GetBase(void);

void SharedMemory_PrintSharedMemoryHex();


#endif // SHAREDMEMORY_H