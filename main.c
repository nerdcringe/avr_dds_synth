#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>
#include "input_reg.h"


/*
	DDS Chord-Playing Piano for the ATtiny85 Microcontroller

	-- Features Direct Digital Synthesis (DDS) for generating multiple waveforms
	-- DDS analog output is made by Pulse-Width Modulation (PWM) and low-pass filtering
	-- PWM uses the Phase-Locked Loop (PLL) as a clock source for faster PWM frequency
	-- Will feature multiple voices that can form chords
	-- Shift register to extend the digital input pins of the microcontroller
*/


/* DDS */
#define NUM_VOICES 3

 // Current wave type selected
uint8_t waveSetting = 0;

// Phase accumulator: keeps track of current phase of wave
volatile unsigned int Acc[NUM_VOICES] = {0};

 // How far to increment the corresponding phase accumulator
volatile unsigned int Jump[NUM_VOICES] = {1760};


#define LUT_SIZE 256

// Precomputed lookup table for a sine wave
// Other waveforms are simpler and don't need to be precomputed
uint8_t sineLUT[LUT_SIZE] = {
	128, 131, 134, 137, 140, 143, 146, 149, 152, 155, 158, 162, 165, 167, 170, 173,
	176, 179, 182, 185, 188, 190, 193, 196, 198, 201, 203, 206, 208, 211, 213, 215,
	218, 220, 222, 224, 226, 228, 230, 232, 234, 235, 237, 238, 240, 241, 243, 244,
	245, 246, 248, 249, 250, 250, 251, 252, 253, 253, 254, 254, 254, 255, 255, 255,
	255, 255, 255, 255, 254, 254, 254, 253, 253, 252, 251, 250, 250, 249, 248, 246,
	245, 244, 243, 241, 240, 238, 237, 235, 234, 232, 230, 228, 226, 224, 222, 220,
	218, 215, 213, 211, 208, 206, 203, 201, 198, 196, 193, 190, 188, 185, 182, 179,
	176, 173, 170, 167, 165, 162, 158, 155, 152, 149, 146, 143, 140, 137, 134, 131,
	128, 124, 121, 118, 115, 112, 109, 106, 103, 100, 97, 93, 90, 88, 85, 82,
	79, 76, 73, 70, 67, 65, 62, 59, 57, 54, 52, 49, 47, 44, 42, 40,
	37, 35, 33, 31, 29, 27, 25, 23, 21, 20, 18, 17, 15, 14, 12, 11,
	10, 9, 7, 6, 5, 5, 4, 3, 2, 2, 1, 1, 1, 0, 0, 0,
	0, 0, 0, 0, 1, 1, 1, 2, 2, 3, 4, 5, 5, 6, 7, 9,
	10, 11, 12, 14, 15, 17, 18, 20, 21, 23, 25, 27, 29, 31, 33, 35,
	37, 40, 42, 44, 47, 49, 52, 54, 57, 59, 62, 65, 67, 70, 73, 76,
	79, 82, 85, 88, 90, 93, 97, 100, 103, 106, 109, 112, 115, 118, 121, 124,
};


// Most recent reading from the parallel-in serial-out shift register (74HC165)
uint16_t incoming = 0;
 

 // Tonic note is the home note for a given key. For example, the tonic in the key of A is just A.
 // Since all the other notes are defined in relation to the tonic note, changing this will change the tonic
uint16_t currentTonic = 32760;

// To change the key, the key up/down buttons are pressed.
// When the number of times a button is pressed matters, debouncing has to be taken into account
// For more information: (https://en.wikipedia.org/wiki/Switch#Contact_bounce)
// In order to debounce the buttons, we will count up while the button is pressed.
// Only when a certain exact number is reached will the key actually change.
uint16_t keyDownPressTime = 0;
uint16_t keyUpPressTime = 0;


// Frequencies for the 12 tonics.
// They are in the high octaves because it is more precise to divide high frequencies than multiply low frequencies.
// For the exact values used: https://en.wikipedia.org/wiki/Piano_key_frequencies
int tonicFrequencies[12] = {
	3520, 3729, 3951, 4186,
	4435, 4699, 4978, 5274,
	5588, 5920, 6272, 6645
};



int main () {

	/* This tutorial shows how the PLL can be used as a clock source for PWM
		http://www.technoblogy.com/show?20W6
	*/
	//Enable 64 MHz PLL and use as source for Timer1
	PLLCSR = 1<<PCKE | 1<<PLLE;

	// Set up Timer/Counter1 for PWM output
	TIMSK = 0;                               // Timer interrupts OFF
	TCCR1 = 1<<PWM1A | 2<<COM1A0 | 1<<CS10;  // PWM A, clear on match, 1:1 prescale
	DDRB |= 1 << DDB1;                      // Enable PWM output pin 

	// Set up Timer/Counter0 for 20kHz interrupt to output samples.
	TCCR0A = 3<<WGM00;                       // Fast PWM
	TCCR0B = 1<<WGM02 | 2<<CS00;             // 1/8 prescale
	TIMSK = 1<<OCIE0A;                       // Enable compare match, disable overflow
	OCR0A = 60;                              // Divide by 61


	// Set up shift register pins
	DDRB &= ~(1 << SR_DATA); // Data is input
	DDRB |= (1 << SR_CLK); // Clock is output
	DDRB |= (1 << SR_LATCH); // Latch is output
  

	/* This tutorial shows how to read analog values without the Arduino library.
		https://www.instructables.com/ATTiny-Port-Manipulation-Part-2-AnalogRead/
	*/

	// Set up ADC input for the pitch tuner
	ADMUX = 0b00100010;   //sets 1.1V IRV, sets ADC3 as input channel, and left adjusts
	ADCSRA = 0b10000011;  //turn on ADC, keep ADC single conversion mode,
						  //and set division factor-8 for 125kHz ADC clock

	
	sei();
	while(1) {	  
		ADCSRA |= (1 << ADSC);   // Obtain the current analog input value
		uint8_t isPlaying = 1;

		uint16_t tonicFreq = tonicFrequencies[currentTonic % 12];
		uint16_t rootFreq;
		uint16_t thirdFreq;
		uint64_t fifthFreq;

		// booleans that determine the structure of the chord
		uint8_t minorThird = 0;
		uint8_t diminishedFifth = 0;
		uint8_t invertThird = 0;
		uint8_t invertFifth = 0;


		// Play the chord that corresponds to the button pressed
		// For each chord, multiply the tonic note by some ratio to get the root frequency of the chord 
		if (incoming & (1 << 6)) { // vii chord
			rootFreq = (((uint32_t)tonicFreq)*15)/16; // root is a major seventh above tonic
			minorThird = 1;
			diminishedFifth = 1;
			invertThird = 1;
			invertFifth = 1;
		} else if (incoming & (1 << 5)) { // vi chord
			rootFreq = (tonicFreq*5)/3; // root is a 6th above tonic
			minorThird = 1;
			invertThird = 1;
			invertFifth = 1;
		} else if (incoming & (1 << 4)) { // V chord
			rootFreq = (tonicFreq*3)/2; // root is a 5th above tonic
			invertThird = 1;
			invertFifth = 1;
		} else if (incoming & (1 << 3)) { // IV chord
			rootFreq = (tonicFreq*4)/3; // root is a 4th above tonic
			invertFifth = 1;
		} else if (incoming & (1 << 2)) { // iii chord
			rootFreq = (tonicFreq*5)/4; // root is a major 3rd above tonic
			minorThird = 1;
		} else if (incoming & (1 << 1)) { // ii chord
			rootFreq = (tonicFreq*9)/8; // root is a major 3rd above tonic
			minorThird = 1;
		} else if (incoming & (1 << 0)) { // I chord
			rootFreq = tonicFreq;
		} else {
			isPlaying = 0;
		}

		// If the swamp major/minor button is pressed, turn a major chord into a minor chord and vice versa
		if (incoming & (1 << 11)) {
			minorThird = !minorThird;
		}

		// Set the other chord tones relative to the root frequency
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

		// Don't play if no chord button is pressed
		if (!isPlaying) {
			rootFreq = 0;
			thirdFreq = 0;
			fifthFreq = 0;
		}

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


		// Sort the frequencies to make the inversions change in the right order.
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
		uint16_t inversionNum = (ADCH/20) + 2;
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
		
		// Assign the frequencies to the phase accumulator jump value
		Jump[0] = lowFreq*2;
		Jump[1] = midFreq*2;
		Jump[2] = highFreq*2;
		

		incoming = readRegister();

		// Key change buttons
		if (incoming & (1 << 7)) {
			keyDownPressTime++;
		} else {
			keyDownPressTime = 0;
		}
		if (keyDownPressTime == 2) { // change keys once only when button pressed for 2 cycles (simple debouncing)
			currentTonic--;
		}
		
		if (incoming & (1 << 8)) {
			keyUpPressTime++;
		} else {
			keyUpPressTime = 0;
		}
		if (keyUpPressTime == 2) {
			currentTonic++;
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
		waveSetting = 0; // 2 bit number that determines which waveform is created
		if (incoming & (1 << 9)) {
			waveSetting += 1 << 1;
		} 
		if (incoming & (1 << 10)) {
			waveSetting += 1;
		}
		switch (waveSetting) { 
			case 0b00: // SINE
				// Access the current sample of the phase accumulator
				sampleHeight += sineLUT[currentAcc];
				break;
				
			case 0b01: // TRIANGLE
				// Increase sample height for the first half, then decrease for the second.
				if (currentAcc < 128) {
					sampleHeight += 2*currentAcc;
				} else {
					sampleHeight += 2*(255-currentAcc);
				}
				break;
				
			case 0b10: // SQUARE
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
	OCR1A = sampleHeight/NUM_VOICES; // set the PWM duty cycle to the average sample height
}