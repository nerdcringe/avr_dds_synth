#include <avr/io.h>

uint8_t getMaxVoices();
void setJump(uint8_t index, int jumpSize);
void setAmplitude(uint8_t index, uint16_t ampl);
void setWaveOutput(uint8_t waveSetting);