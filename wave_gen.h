#include <avr/io.h>
#include <avr/pgmspace.h>

uint8_t getMaxVoices();
void setJump(uint8_t index, int jumpSize);
void setAmplitude(uint8_t index, uint8_t ampl);
void setWaveOutput(uint8_t waveSetting);