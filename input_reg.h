#include <avr/io.h>

#define NUM_BITS 16


/* Shift Register */
// This project uses an SN74HC165 shift register to expand the digital input pins available
#define SR_DATA PB3
#define SR_CLK PB2
#define SR_LATCH PB0


uint16_t readRegister();
