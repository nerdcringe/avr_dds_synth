#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>
#include "input_reg.h"
#include "low_level.h"
#include "chord_queue.h"
#include "wave_gen.h"


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

 // Tonic note is the home note for a given key. For example, the tonic in the key of A is just A.
 // Every note in a key is defined in relation to the tonic, so changing the tonic changes the whole key.
uint16_t keyIndex = 32767;
int tonicFreqs[12] = {
	3520, 3729, 3951, 4186,
	4435, 4699, 4978, 5274,
	5588, 5920, 6272, 6645
};


int main () {
	initPWM();
	initAnalogIn();
	initInputRegister();
	sei(); // Enable interrupts
	
	while(1) {
		updateAnalogIn();
		readInputRegister();
		
		uint16_t tonicFreq = tonicFreqs[keyIndex % 12];

		for (uint16_t buttonIndex = 0; buttonIndex < 7; buttonIndex++) {
			uint16_t rootFreq = 0;
			uint16_t thirdFreq = 0;
			uint16_t fifthFreq = 0;

			// booleans that determine the structure of the chord
			uint8_t minorThird = 0;
			uint8_t diminishedFifth = 0;

			if (getInputStarted(buttonIndex)) {
				
				switch (buttonIndex) {
				case vii_CHORD:
					rootFreq = (((uint64_t)tonicFreq)*15)/8;
					minorThird = 1;
					diminishedFifth = 1;
				break;
				case vi_CHORD:
					rootFreq = (tonicFreq*5)/3;
					minorThird = 1;
				break;
				case V_CHORD:
					rootFreq = (tonicFreq*3)/2;
				break;
				case IV_CHORD:
					rootFreq = (tonicFreq*4)/3;
				break;
				case iii_CHORD:
					rootFreq = (tonicFreq*5)/4;
					minorThird = 1;
				break;
				case ii_CHORD:
					rootFreq = (tonicFreq*9)/8;
					minorThird = 1;
				break;
				case I_CHORD:
					rootFreq = tonicFreq; 
				default:
				break;
				}

				if (getInputState(MAJ_MIN_SWAP)) {
					minorThird = !minorThird;
				}
				if (minorThird) {
					thirdFreq = (rootFreq*6)/5; // minor 3rd above root
				} else {
					thirdFreq = (rootFreq*5)/4; // major 3rd above root
				}

				if (diminishedFifth) {
					fifthFreq = (((uint64_t)rootFreq)*45)/32; // diminished 5th above root
				} else {
					fifthFreq = (rootFreq*3)/2; // perfect 5th above root
				}
				//playChord(rootFreq, thirdFreq, fifthFreq, buttonIndex);
			
				// If any note is an octave above the tonic (2x higher frequency), then bring it down an octave (divide frequency by 2)
				if (rootFreq >= tonicFreq*1.95) { // value to check is *1.95 to account for integer division rounding
					rootFreq /= 2;
				}
				if (thirdFreq >= tonicFreq*1.95) {
					thirdFreq /= 2;
				}
				if (fifthFreq >= tonicFreq*1.95) {
					fifthFreq /= 2;
				}

				// Sort the frequencies to make the inversion cycle change in the right order.
				uint16_t lowFreq = fmin(rootFreq, fmin(thirdFreq, fifthFreq));
				uint16_t highFreq = fmax(rootFreq, fmax(thirdFreq, fifthFreq));
				uint16_t midFreq;
				if (rootFreq > lowFreq && rootFreq < highFreq) {
					midFreq = rootFreq;
				} else if (thirdFreq > lowFreq && thirdFreq < highFreq) {
					midFreq = thirdFreq;
				} else {
					midFreq = fifthFreq;
				}

				// As the analog input goes from 0 -> 256, each pitch goes down an octave one-by-one in a cycle.
				// The highest pitch goes down each time, which is why they needed to be sorted.;
				uint16_t inversionNum = (ADCH/24);
				for (int i = 0; i < inversionNum; i++) {
					if (i % 3 == 2) {
						lowFreq /= 2;
					}
					if (i % 3 == 1) {
						midFreq /= 2;
					}
					if (i % 3 == 0) {
						highFreq /= 2;
					}
				}
				
				playChord(lowFreq, midFreq, highFreq, buttonIndex);
			}
		}

		for (int i = 0; i < 7; i++) {
			if (getInputState(i) == 0) {
				releaseChord(i);
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