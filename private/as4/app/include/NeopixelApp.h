#ifndef NEOPIXELAPP_H
#define NEOPIXELAPP_H

#include <stdint.h>

#define Blue        0x00009000
#define LightBlue   0x00006000
#define Green       0x10000000
#define LightGreen  0x05000000
#define Orange      0x4dff0000
#define Red         0x00b00000
#define LightRed    0x000f0000

/**
 * @brief Initialize resources for controlling the NeoPixel (shared memory, etc.).
 *        Must be called before using other NeopixelApp_* functions.
 */
void NeopixelApp_Init(void);

/**
 * @brief Cleanup resources (unmap shared memory, etc.).
 *        After this call, no further NeoPixel functions should be used.
 */
void NeopixelApp_Cleanup(void);

/**
 * @brief Set all 8 LEDs using an array of 8 32-bit colors.
 *
 * @param colors  An array of exactly 8 values. Each is a 32-bit color for one LED.
 *                The exact bit format depends on how your R5 code interprets them (e.g. GRB).
 */
void NeopixelApp_SetAll(const uint32_t colors[8]);

/**
 * @brief Set one LED at a given index to a specific color.
 *
 * @param color  32-bit color value.
 * @param index  Index of LED (0..7). Out-of-range checks are optional.
 */
void NeopixelApp_SetOne(uint32_t color, int index);

/**
 * @brief A sample “blue chase” animation that quickly lights one LED in blue,
 *        moving from index 0 to 7, then off.
 */
void NeopixelApp_HitAnimation(void);

/**
 * @brief A sample “red chase” animation that quickly lights one LED in red,
 *        moving from index 0 to 7, then off.
 */
void NeopixelApp_MissAnimation(void);


#endif // NEOPIXELAPP_H
