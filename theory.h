#include <avr/io.h>


void getMainFreqs(uint8_t keyIndex, uint8_t chordNum, uint8_t swapMajMin, uint8_t inversionNum,
    uint16_t* lowFreq, uint16_t* midFreq, uint16_t* highFreq);