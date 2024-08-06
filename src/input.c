
#include "SR_74HC165.h"
#include "bitwise.h"

#define NUM_CHORD_BUTTONS 7

uint8_t pressDurations[SR_NUM_BITS] = {0};

void initInput(uint8_t data, uint8_t clock, uint8_t latch) {
    initInputRegister(data, clock, latch);
}

void updateInput() {

    uint16_t incoming = readInputRegister();

    for (int i = 0; i < SR_NUM_BITS; i++) {
        if (checkBit(incoming, i)) {

			// Increment press duration while pressed
			if (pressDurations[i] < 16) {
				pressDurations[i]++;
			}

        } else {
			// Reset press duration when released
			pressDurations[i] = 0;
        }
    }
}


// Return 1 while the input state is 1
uint8_t getInputState(uint8_t index) {
	return pressDurations[index] > 0;
}

// Only return 1 when the input state just became 1
uint8_t getInputStarted(uint8_t index) {
	return pressDurations[index] == 10;
}