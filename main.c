#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>
#include "input_reg.h"
#include "low_level.h"
#include "wave_gen.h"
#include "theory.h"


/*
	DDS Chord-Playing Piano for the ATtiny85 Microcontroller

	-- Features Direct Digital Synthesis (DDS) for generating multiple waveforms
	-- DDS analog output is made by Pulse-Width Modulation (PWM) and low-pass filtering
	-- PWM uses the Phase-Locked Loop (PLL) as a clock source for faster PWM frequency
	-- Will feature multiple voices that can form chords
	-- Shift register to extend the digital input pins of the microcontroller
*/


 // Current wave type selected
volatile uint8_t waveSetting = 0;
uint16_t currentkeyIndex = 32767;
uint8_t lastInversionNum = 0;
uint8_t inversionChangeTime = 0;


int main () {
	initPWM();
	initAnalogIn();
	initInputRegister();
	sei(); // Enable interrupts
	
	while(1) {
		updateAnalogIn();
		readInputRegister();

		uint8_t inversionNum = (ADCH/28);

		if (inversionNum != lastInversionNum) {
			inversionChangeTime = 0;
		}
		if (inversionChangeTime < 254) {
			inversionChangeTime++;
		}

		uint8_t playing = 0;
		for (uint16_t buttonIndex = 0; buttonIndex < 7; buttonIndex++) {
			if (getInputState(buttonIndex)) {
				playing = 1;
				uint16_t lowFreq, midFreq, highFreq;

				getMainFreqs(currentkeyIndex, buttonIndex, getInputState(MAJ_MIN_SWAP),
							inversionNum, &lowFreq, &midFreq, &highFreq);
				
				setJump(0, lowFreq);
				setJump(1, midFreq);
				setJump(2, highFreq);
				break;
			}
		}
		if (!playing && !getInputState(SUSTAIN_MODE)) {
			for (int i = 0; i < MAX_VOICES; i++) {
				setJump(i, 0);
			}
		}

		// Change the key once when the key change button starts being pressed
		if (getInputStarted(8)) {
			currentkeyIndex--;
		}
		if (getInputStarted(9)) {
			currentkeyIndex++;
		}

		if (getInputState(WAVE1)) {
			if (getInputState(WAVE0)) {
				waveSetting = 0;
			} else {
				waveSetting = 1;
			}
		} else {
			if (getInputState(WAVE0)) {
				waveSetting = 2;
			} else {
				waveSetting = 3;
			}
		}
		
		lastInversionNum = inversionNum;
	}
}


// Interrupt to create waveforms with pulse-width-modulation
ISR(TIMER0_COMPA_vect) {
	setWaveOutput(waveSetting);
}