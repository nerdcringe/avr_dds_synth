#include <avr/io.h>

#define NUM_BITS 16


/* Shift Register
    - This project uses an SN74HC165 shift register to expand the digital input pins available
    - The shift register in Parallel-In, Serial-Out (PISO), taking the state of many inputs at once
    and sending them one-by-one to the microcontroller
    - Instead of using 16 I/O pins for 16 buttons and switches, it only takes 3 (data, clock, and latch/load)
*/

#define SR_DATA PB3
#define SR_CLK PB2
#define SR_LATCH PB0


#define MAJ_MIN_SWAP 7

#define KEY_UP 8
#define KEY_DOWN 9
#define WAVE1 10
#define WAVE0 11
#define SINGLE_NOTE 12


void initInputRegister();
void readInputRegister();

uint8_t getInputState(uint8_t registerPin);
uint8_t getInputStarted(uint8_t registerPin);