#include <avr/io.h>
#include <avr/interrupt.h>
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
#define NUM_VOICES 1

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



uint16_t incoming = 0;





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
		ADCSRA |= (1 << ADSC);   //start conversion
		Jump[0] = ADCH*16 ;

		incoming = readRegister();
		
	}
}



// Interrupt to set the PWM duty cycle to the correct amplitude
ISR(TIMER0_COMPA_vect) {
	
	
	for (int i = 0; i < NUM_VOICES; i++) {
		Acc[i] = Acc[i] + Jump[i];
		
		unsigned int amplitude = 0;
		
		// shift by 8 bits (convert from 16 bit to 8 bit)
		// 2^8 is 256 so 8-bits can only represent numbers from 0-255
		uint8_t currentAcc = Acc[i] >> 8;
		waveSetting = 0; // 2 bit number that determines which waveform is created
		if (incoming & (1 << 6)) {
			waveSetting += 1 << 1;
		}
		if (incoming & (1 << 7)) {
			waveSetting += 1;
		}
		switch (waveSetting) { 
			case 0b00: // SINE
				amplitude += sineLUT[currentAcc];
				break;
				
			case 0b01: // TRIANGLE
				if (currentAcc < 128) {
					amplitude += 2*currentAcc;
				} else {
					amplitude += 2*(255-currentAcc);
				}
				break;
				
			case 0b10: // SQUARE
				if (currentAcc < 128) {
					amplitude += 255;
				}
				break; 
			
			case 0b11: // SAW
				amplitude += currentAcc;
				break;
		}
		OCR1A = amplitude; // set the PWM duty cycle to the amplitude of the wave
	}
}