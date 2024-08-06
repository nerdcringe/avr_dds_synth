#include <avr/io.h>

void initPWM();
void setDutyCycle(uint8_t dutyCycle);

void initAnalogIn();
void updateAnalogIn();
uint8_t readAnalogIn();
uint8_t analogInChanged();


