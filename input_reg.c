#include "input_reg.h"


uint16_t readRegister() {
	// Step 1: Sample
	PORTB &= ~(1 << SR_LATCH);
	for (int i = 0; i < 5; i++) {}
	PORTB |= (1 << SR_LATCH);
	for (int i = 0; i < 5; i++) {}


	uint16_t incoming = 0;
	for (int i = 0; i < NUM_BITS; i++) {
		PORTB &= ~(1 << SR_CLK);

		if (PINB & (1 << SR_DATA)) {
		  incoming |= 1 << i;
		}

		PORTB |= (1 << SR_CLK);
	}
	return incoming;
}
