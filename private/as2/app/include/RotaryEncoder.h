#ifndef ROTARYENCODER_H
#define ROTARYENCODER_H

void RotaryEncoder_init(void);
void RotaryEncoder_handleChange(int counter);
int RotartEncoder_currentVolume(void);
void RotaryEncoder_cleanup(void);

#endif