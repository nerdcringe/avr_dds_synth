#include "input_reg.h"
#include "low_level.h"

volatile uint8_t currentReading = 0; // Nth bit is whether the Nth button is being held
volatile uint8_t pressDurations[NUM_BITS] = {0};


void initInputRegister() {
	DDRB = clearBit(DDRB, SR_DATA); // Data is input
	DDRB = setBit(DDRB, SR_CLK); // Clock is outputs
	DDRB = setBit(DDRB, SR_LATCH); // Latch is output
}

// Read from the SN74HC165 shift register
void readInputRegister() {
	// Pulse the latch to store the current button readings
	PORTB = clearBit(PORTB, SR_LATCH);
	for (int i = 0; i < 2; i++) {}
	PORTB = setBit(PORTB, SR_LATCH);
	for (int i = 0; i < 2; i++) {}
 
	// Read every bit one-by-one, pulsing the clock each time to shift the next bit in
	uint16_t incoming = 0;
	for (int i = 0; i < NUM_BITS; i++) {
		// set clock to 0 to get next bit
		PORTB = clearBit(PORTB, SR_CLK);
		// Read current bit
		if (PINB & (1 << SR_DATA)) {
			incoming |= 1 << i;
			// Increment press duration while pressed
			if (pressDurations[i] < 10) {
				pressDurations[i]++;
			}
		} else {
			// Reset press duration when released
			pressDurations[i] = 0;
		}
		PORTB = setBit(PORTB, SR_CLK); // set clock to 1 before next read
	}
	currentReading = incoming;
}

uint8_t getInputStarted(uint8_t index) {
	return pressDurations[index] == 1;
}

uint8_t getInputState(uint8_t index) {
	return currentReading & (1 << index);
}