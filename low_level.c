#include <avr/io.h>

uint8_t setBit(uint16_t bitString, uint8_t bitToSet) {
    return bitString | (1 << bitToSet);
}

uint8_t clearBit(uint16_t bitString, uint8_t bitToClear) {
    return bitString & ~(1 << bitToClear);
}

uint8_t checkBit(uint16_t bitString, uint8_t bitToCheck) {
	return (bitString & (1 << bitToCheck)) > 0;
}


void initPWM() {
	/* This tutorial shows how the PLL can be used as a clock source for PWM
		It guided much of the register settings used in this method
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
	OCR0A = 60;                              // Divide by 61, giving us increments of about 1 Hz
}

void setDutyCycle(uint8_t dutyCycle) {
	OCR1A = dutyCycle;
}


void initAnalogIn() {
	/* This tutorial shows how to read analog values without the Arduino library.
		https://www.instructables.com/ATTiny-Port-Manipulation-Part-2-AnalogRead/
	*/

	// Set up ADC input for the inversion potentiometer
	ADMUX = 0b00100010;   //sets 5V reference, sets ADC3 as input channel, and left adjusts
	ADCSRA = 0b10000011;  //turn on ADC, keep ADC single conversion mode,
						  //and set division factor-8 for 125kHz ADC clock
}

void updateAnalogIn() { 
	ADCSRA = setBit(ADCSRA, ADSC);// Convert the current analog input value
}

uint8_t readAnalogIn() {
	return ADCH;
}
