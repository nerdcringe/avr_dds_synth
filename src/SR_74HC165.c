#include "SR_74HC165.h"
#include "bitwise.h"

uint8_t data;
uint8_t clock;
uint8_t latch;

void initInputRegister(uint8_t _data, uint8_t _clock, uint8_t _latch) {
	data = _data;
	clock = _clock;
	latch = _latch;
	DDRB = clearBit(DDRB, data); // Data is input
	DDRB = setBit(DDRB, clock); // Clock is outputs
	DDRB = setBit(DDRB, latch); // Latch is output
}

// Read from the SN74HC165 shift register
uint16_t readInputRegister() {

	// Pulse the latch to shift in the current input values
	PORTB = clearBit(PORTB, latch);
	for (int i = 0; i < 2; i++) {}
	PORTB = setBit(PORTB, latch);
	for (int i = 0; i < 2; i++) {}
 
	// Read every bit one-by-one, pulsing the clock each time to shift the next bit in
	uint16_t incoming = 0;
	for (int i = 0; i < SR_NUM_BITS; i++) {
		// set clock to 0 to get next bit
		PORTB = clearBit(PORTB, clock);

		// Read current bit
		if (checkBit(PINB, data)) {
			incoming = setBit(incoming, i);
		}
		PORTB = setBit(PORTB, clock); // set clock to 1 before next read
	}
	return incoming;
}