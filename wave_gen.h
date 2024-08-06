#include <avr/io.h>

// Maximum number of simultaneous frequencies able to be played ad once
#define MAX_VOICES 3

void setJump(uint8_t index, int jumpSize);
void setAmplitude(uint8_t index, uint16_t ampl);
void setWaveOutput(uint8_t waveSetting);