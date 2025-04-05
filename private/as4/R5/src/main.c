#include <stdio.h>
#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <string.h>


// Include your sharedDataLayout.h file
#include "sharedDataLayout.h"

//
// For simplicity, we define a pointer to the start of BTCM memory.
// (0x00000000 is how the R5 sees it; see your TRM or previous code.)
//
#define SHARED_MEM_BTCM_START  (0x00000000) // TRM p848
#define BASE                   ((void*)(SHARED_MEM_BTCM_START))

//
// NeoPixel Timing Macros (Adapt to your timing needs):
//

// Reset must be at least ~50us; we'll do 60us
#define NEO_RESET_NS      60000

volatile int junk_delay = 0;
#define DELAY_350_NS() {/* short delay */}
#define DELAY_600_NS() {for (junk_delay=0; junk_delay<9 ;junk_delay++);}  
#define DELAY_700_NS() {for (junk_delay=0; junk_delay<16;junk_delay++);}  
#define DELAY_800_NS() {for (junk_delay=0; junk_delay<23;junk_delay++);}  

#define DELAY_NS(ns) do {int target = k_cycle_get_32() + k_ns_to_cyc_near32(ns); \
	while(k_cycle_get_32() < target) ; } while(false)

// You can also do k_busy_wait() or a more precise approach with cycle counts if you prefer:
#define NEO_DELAY_ONE_ON()     DELAY_700_NS()
#define NEO_DELAY_ONE_OFF()    DELAY_600_NS()
#define NEO_DELAY_ZERO_ON()    DELAY_350_NS()
#define NEO_DELAY_ZERO_OFF()   DELAY_800_NS()
#define NEO_DELAY_RESET()  DELAY_NS(NEO_RESET_NS)

// Our NeoPixel chain length and devicetree alias
#define NEO_NUM_LEDS    8

// Devicetree alias "neopixel" must match your hardware setup
#define NEOPIXEL_NODE DT_ALIAS(neopixel)
static const struct gpio_dt_spec neopixel = GPIO_DT_SPEC_GET(NEOPIXEL_NODE, gpios);

//
// Helper function to initialize GPIO pins with Zephyr
//
static void initialize_gpio(const struct gpio_dt_spec *spec, gpio_flags_t flags)
{
    if (!device_is_ready(spec->port)) {
        printf("ERROR: GPIO device not ready.\n");
        exit(EXIT_FAILURE);
    }
    if (gpio_pin_configure_dt(spec, flags)) {
        printf("ERROR: Could not configure pin.\n");
        exit(EXIT_FAILURE);
    }
}

//
// Function to write out an array of 8 colors to NeoPixel
//   color[] format: 32 bits each: (G R B W) or (R G B) depending on your usage.
//   For standard WS2812, you typically send G-R-B order, each 8 bits.
static void neopixel_send_colors(const uint32_t color[NEO_NUM_LEDS])
{
    gpio_pin_set_dt(&neopixel, 0);
		DELAY_NS(NEO_RESET_NS);

		for(int j = 0; j < NEO_NUM_LEDS; j++) {
			for(int i = 31; i >= 0; i--) {
				if(color[j] & ((uint32_t)0x1 << i)) {
					gpio_pin_set_dt(&neopixel, 1);
					NEO_DELAY_ONE_ON();
					gpio_pin_set_dt(&neopixel, 0);
					NEO_DELAY_ONE_OFF();
				} else {
					gpio_pin_set_dt(&neopixel, 1);
					NEO_DELAY_ZERO_ON();
					gpio_pin_set_dt(&neopixel, 0);
					NEO_DELAY_ZERO_OFF();
				}
			}
		}

		gpio_pin_set_dt(&neopixel, 0);
		NEO_DELAY_RESET();
}

int main(void)
{
    printf("Starting R5 main. Using shared memory at 0x%08x\n", SHARED_MEM_BTCM_START);

    // Initialize the NeoPixel GPIO pin
    initialize_gpio(&neopixel, GPIO_OUTPUT_ACTIVE);

    // Local array of 8 colors
    uint32_t ledColors[NEO_NUM_LEDS];

    // Clear the flag
    setSharedMem_uint8(BASE, LED_FLAG_OFFSET, 0);


    memset(ledColors, 0, sizeof(ledColors));

    while (1) {
        // Check if flag is set
        uint8_t flagVal = getSharedMem_uint8(BASE, LED_FLAG_OFFSET);
        if (flagVal != 0) {
            // 1) Read the 8 color values from shared memory
            for (int i = 0; i < NEO_NUM_LEDS; i++) {
                ledColors[i] = getSharedMem_uint32(BASE, LED_0_OFFSET + i * sizeof(uint32_t));
            }

			// 2) Clear the flag to let Linux know we’ve consumed the data
            setSharedMem_uint8(BASE, LED_FLAG_OFFSET, 0);

            // 3) Send them out to the NeoPixel strip
            neopixel_send_colors(ledColors);
            printf("LED array updated from shared memory.\n");
        }
    }
}
