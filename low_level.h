#include <avr/io.h>

uint16_t setBit(uint16_t bitString, uint8_t bitToSet);
uint16_t clearBit(uint16_t bitString, uint8_t bitToClear);
uint8_t checkBit(uint16_t bitString, uint8_t bitToCheck);

void initPWM();
void setDutyCycle();

void initAnalogIn();
void updateAnalogIn();
uint8_t readAnalogIn();


