// This module provides functionality to initialize, update, and clean up an LCD screen. 
// It uses a frame buffer to manage the display content and supports drawing text and other graphical elements on the screen.

#ifndef _USE_LCD_H_
#define _USE_LCD_H_

// Initializes the LCD module, sets up the hardware, and allocates memory for the frame buffer.
void LCD_init();

// Cleans up resources, clears the LCD screen, turns off the backlight, and frees the frame buffer memory.
void LCD_cleanup();

// Updates the LCD screen with the provided message, splitting it into lines and displaying each line sequentially.
void LCD_updateScreen(char* message);

#endif