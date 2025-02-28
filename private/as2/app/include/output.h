// This module periodically collects and displays statistics about light sampling, 
// including the number of samples, dips, and timing information. 
// It uses an LCD screen for visual output and prints detailed information to the console.

#ifndef _OUTPUT_H_
#define _OUTPUT_H_

// Initializes the module, sets up the LCD screen, and starts a background thread to handle periodic output.
void Output_init(void);

// Cleans up resources, signals the background thread to shut down, and joins the thread.
void Output_cleanup(void);

#endif