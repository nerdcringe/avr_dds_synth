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
uint8_t keyIndex = 5;
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
		
		uint16_t tonicFreq = tonicFreqs[keyIndex];

		if (getInputStarted(6)) { // vii chord
		// root is a major seventh above tonic
			playChord((((uint32_t)tonicFreq)*15)/8, 6);
		}
		if (getInputStarted(5)) { // vi chord
			playChord((tonicFreq*5)/3, 5); // 6th above tonic
		}
		if (getInputStarted(4)) { // V chord
			playChord((tonicFreq*3)/2, 4); // 5th above tonic
		}
		if (getInputStarted(3)) { // IV chord
			playChord((tonicFreq*4)/3, 3); //  4th above tonic
		}
		if (getInputStarted(2)) { // iii chord
			playChord((tonicFreq*5)/4, 2); // major 3rd above tonic
		}
		if (getInputStarted(1)) { // ii chord
			playChord((tonicFreq*9)/8, 1); // minor 2nd above tonic
		}
		if (getInputStarted(0)) { // I chord
			playChord(tonicFreq, 0); // root is tonic
		}
		
		/*
		for (int i = 0; i < 7; i++) {
			if (getInputState(i) == 0) {
				releaseChord(i);
			}
		}
		manageChords();

		// CHANGE WHEN CHORDS HAVE MULTIPLE NOTES (specifically change getChord's index)
		for (int i = 0; i < getMaxVoices(); i++) {
			setJump(i, getChord(i).freq0/4);
			setAmplitude(i, getChord(i).amplitude);
		}*/

		// Change the key once when the key change button starts being pressed
		if (getInputStarted(7)) {
			keyIndex--;
		}
		if (getInputStarted(8)) {
			keyIndex++;
		}
		if (keyIndex > 11) {
			keyIndex = 0;
		}
		if (keyIndex < 0) {
			keyIndex = 11;
		}

		if (getInputState(9)) {
			if (getInputState(10)) {
				waveSetting = 0b00;
			} else {
				waveSetting = 0b01;
			}
		} else {
			if (getInputState(10)) {
				waveSetting = 0b10;
			} else {
				waveSetting = 0b11;
			}
		} 
		
	}
}


// Interrupt to create waveforms with pulse-width-modulation
ISR(TIMER0_COMPA_vect) {
	setWaveOutput(waveSetting);
}