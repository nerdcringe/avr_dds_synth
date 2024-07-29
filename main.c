#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <math.h>
#include "input_reg.h"
#include "low_level.h"
#include "chord_queue.h"
//#include "theory.h"


/*
	DDS Chord-Playing Piano for the ATtiny85 Microcontroller

	-- Features Direct Digital Synthesis (DDS) for generating multiple waveforms
	-- DDS analog output is made by Pulse-Width Modulation (PWM) and low-pass filtering
	-- PWM uses the Phase-Locked Loop (PLL) as a clock source for faster PWM frequency
	-- Will feature multiple voices that can form chords
	-- Shift register to extend the digital input pins of the microcontroller
*/


/* DDS */
#define NUM_VOICES (NOTES_PER_CHORD * MAX_CHORDS)

 // Current wave type selected
volatile uint8_t waveSetting = 0;

// Phase accumulator: keeps track of current phase of wave
volatile unsigned int Acc[NUM_VOICES] = {0};

 // How far to increment the corresponding phase accumulator
volatile unsigned int Jump[NUM_VOICES] = {0};


#define LUT_SIZE 128

// Precomputed lookup table for a sine wave
// Other waveforms are simpler and don't need to be precomputed
const uint8_t sineLUT[LUT_SIZE] PROGMEM = {
	128, 134, 140, 146, 152, 158, 165, 170, 176, 182, 188, 193, 198, 203, 208, 213,
	218, 222, 226, 230, 234, 237, 240, 243, 245, 248, 250, 251, 253, 254, 254, 255,
	255, 255, 254, 254, 253, 251, 250, 248, 245, 243, 240, 237, 234, 230, 226, 222,
	218, 213, 208, 203, 198, 193, 188, 182, 176, 170, 165, 158, 152, 146, 140, 134,
	128, 121, 115, 109, 103, 97, 90, 85, 79, 73, 67, 62, 57, 52, 47, 42,
	37, 33, 29, 25, 21, 18, 15, 12, 10, 7, 5, 4, 2, 1, 1, 0,
	0, 0, 1, 1, 2, 4, 5, 7, 10, 12, 15, 18, 21, 25, 29, 33,
	37, 42, 47, 52, 57, 62, 67, 73, 79, 85, 90, 97, 103, 109, 115, 121
};


// To change the key, the key up/down buttons are pressed.
// When the number of times a button is pressed matters, debouncing has to be taken into account
// For more information: (https://en.wikipedia.org/wiki/Switch#Contact_bounce)
// In order to debounce the buttons, we will count up while the button is pressed.
// Only when a certain exact number is reached will the key actually change.
uint16_t keyDownPressTime = 0;
uint16_t keyUpPressTime = 0;



 // Tonic note is the home note for a given key. For example, the tonic in the key of A is just A.
 // Every note in a key is defined in relation to the tonic, so changing the tonic changes the whole key.
uint8_t keyIndex = 5;
int tonicFreqs[12] = {
	3520, 3729, 3951, 4186,
	4435, 4699, 4978, 5274,
	5588, 5920, 6272, 6645
};


uint8_t volumes[NUM_VOICES] = {0};


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
			insertChord((((uint32_t)tonicFreq)*15)/8);
		}
		if (getInputStarted(5)) { // vi chord
			insertChord((tonicFreq*5)/3); // 6th above tonic
		}
		if (getInputStarted(4)) { // V chord
			insertChord((tonicFreq*3)/2); // 5th above tonic
		}
		if (getInputStarted(3)) { // IV chord
			insertChord((tonicFreq*4)/3); //  4th above tonic
		}
		if (getInputStarted(2)) { // iiichord
			insertChord((tonicFreq*5)/4); // major 3rd above tonic
		}
		if (getInputStarted(1)) { // ii chord
			insertChord((tonicFreq*9)/8); // minor 2nd above tonic
		}
		if (getInputStarted(0)) { // I chord
			insertChord(tonicFreq); // root is tonic
		}

		// Four one-note chords can be played at once (for testing)
		Jump[0] = getChord(0) >> 2;
		Jump[1] = getChord(1) >> 2;
		Jump[2] = getChord(2) >> 2;
		Jump[3] = getChord(3) >> 2;


		// Change the key once when the key change button starts being pressed
		if (getInputStarted(7)) {
			keyIndex--;
		}
		if (getInputStarted(8)) {
			keyIndex++;
		}
		if (keyIndex > 11) {
			keyIndex = 0;
		} else if (keyIndex < 0) {
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


// Interrupt to set the PWM duty cycle to the correct amplitude
ISR(TIMER0_COMPA_vect) {
	uint16_t sampleHeight = 0;

	for (int i = 0; i < NUM_VOICES; i++) {
		Acc[i] = Acc[i] + Jump[i];

		// shift by 8 bits (convert from 16 bit to 8 bit)
		// 2^8 is 256 so 8-bits can only represent numbers from 0-255
		uint8_t currentAcc = Acc[i] >> 8;
		
		switch (waveSetting) { 
			case 0b00: // SINE
				// Access the current sample of the phase accumulator
				sampleHeight += pgm_read_byte(&sineLUT[currentAcc/2]);
				break;
				
			case 0b10: // TRIANGLE
				// Increase sample height for the first half, then decrease for the second.
				if (currentAcc < 128) {
					sampleHeight += 2*currentAcc;
				} else {
					sampleHeight += 2*(255-currentAcc);
				}
				break;
				
			case 0b01: // SQUARE
				// Max out sample height for the first half, then don't add anything for the second half
				if (currentAcc < 128) {
					sampleHeight += 255;
				}
				break; 
			
			case 0b11: // SAW
				// Increase the sample height until the phase accumulator overflows
				sampleHeight += currentAcc;
				break;
		}
	}
	setDutyCycle(sampleHeight/NUM_VOICES); // set the PWM duty cycle to the average sample height
}