#include <avr/io.h>


/* Shift Register
    - This project uses an SN74HC165 shift register to expand the digital input pins available
    - The shift register in Parallel-In, Serial-Out (PISO), taking the state of many inputs at once
    and sending them one-by-one to the microcontroller
    - Instead of using 16 I/O pins for 16 buttons and switches, it only takes 3 (data, clock, and latch/load)
*/


#define SR_NUM_BITS 16


void initInputRegister(uint8_t data, uint8_t clock, uint8_t latch);
uint16_t readInputRegister();