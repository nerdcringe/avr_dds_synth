#include <avr/io.h>

void initInput();
void updateInput();
uint8_t getInputState(uint8_t registerPin);
uint8_t getInputStarted(uint8_t registerPin);