#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>
#include "input_reg.h"
#include "low_level.h"
#include "wave_gen.h"
#include "chord_queue.h"
#include "theory.c"


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
uint16_t keyIndex = 32767;

int main () {
	initPWM();
	initAnalogIn();
	initInputRegister();
	sei(); // Enable interrupts
	
	while(1) {
		updateAnalogIn();
		readInputRegister();

		for (uint16_t buttonIndex = 0; buttonIndex < 7; buttonIndex++) {
			if (getInputStarted(buttonIndex)) {

				uint16_t lowFreq, midFreq, highFreq;
				uint8_t inversionNum = (ADCH/24);

				getMainFreqs(keyIndex, buttonIndex, getInputState(MAJ_MIN_SWAP),
							inversionNum, &lowFreq, &midFreq, &highFreq);

				playChord(lowFreq, midFreq, highFreq, buttonIndex);

			} else if (getInputState(buttonIndex) == 0) {
				releaseChord(buttonIndex);
			}
		}
		manageChords();

		uint8_t voiceIndex = 0;
		for (uint8_t chordIndex = 0; chordIndex < getNumChords(); chordIndex++) {
			for (uint8_t freqIndex = 0; freqIndex < FREQS_PER_CHORD; freqIndex++) {
				
				chord_t chord = getChord(chordIndex);
				
				uint16_t frequency = chord.freq0;
				if (freqIndex == 1)  {
					frequency = chord.freq1;
				}
				if (freqIndex == 2) {
					frequency = chord.freq2;
				}
				setJump(voiceIndex, frequency);
				setAmplitude(voiceIndex, chord.amplitude);
				voiceIndex++;
			}
		}

		// Change the key once when the key change button starts being pressed
		if (getInputStarted(8)) {
			keyIndex--;
		}
		if (getInputStarted(9)) {
			keyIndex++;
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
		
	}
}


// Interrupt to create waveforms with pulse-width-modulation
ISR(TIMER0_COMPA_vect) {
	setWaveOutput(waveSetting);
}