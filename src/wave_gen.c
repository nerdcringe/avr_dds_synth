#include "wave_gen.h"
#include "low_level.h"

volatile uint8_t numActiveVoices = 0;


// Phase accumulator: keeps track of current phase of wave
volatile unsigned int accumulator[MAX_VOICES] = {0};

 // How far to increment the corresponding phase accumulator
volatile unsigned int jump[MAX_VOICES] = {0};


#define LUT_SIZE 128

// Precomputed lookup table for a sine wave
// Other waveforms are simpler and don't need to be precomputed
const uint8_t sineLUT[LUT_SIZE] = {
	128, 134, 140, 146, 152, 158, 165, 170, 176, 182, 188, 193, 198, 203, 208, 213,
	218, 222, 226, 230, 234, 237, 240, 243, 245, 248, 250, 251, 253, 254, 254, 255,
	255, 255, 254, 254, 253, 251, 250, 248, 245, 243, 240, 237, 234, 230, 226, 222,
	218, 213, 208, 203, 198, 193, 188, 182, 176, 170, 165, 158, 152, 146, 140, 134,
	128, 121, 115, 109, 103, 97, 90, 85, 79, 73, 67, 62, 57, 52, 47, 42,
	37, 33, 29, 25, 21, 18, 15, 12, 10, 7, 5, 4, 2, 1, 1, 0,
	0, 0, 1, 1, 2, 4, 5, 7, 10, 12, 15, 18, 21, 25, 29, 33,
	37, 42, 47, 52, 57, 62, 67, 73, 79, 85, 90, 97, 103, 109, 115, 121
};


void setJump(uint8_t index, int jumpSize) {
    jump[index] = jumpSize;
}


void setWaveOutput(uint8_t waveSetting) {
    uint16_t totalSampleHeight = 0;
	for (int i = 0; i < MAX_VOICES; i++) {
		accumulator[i] = accumulator[i] + jump[i];

		// shift by 8 bits (convert from 16 bit to 8 bit)
		// 2^8 is 256 so 8-bits can only represent numbers from 0-255
		uint8_t currentAcc = accumulator[i] >> 8;

        uint32_t sampleHeight = 0;
		
		switch (waveSetting) { 
			case 0: // SINE
				// Access the current sample of the phase accumulator
				sampleHeight = sineLUT[currentAcc/2];//pgm_read_byte(&sineLUT[currentAcc/2]);
				break;

			case 1: // TRIANGLE
				// Increase sample height for the first half, then decrease for the second.
				if (currentAcc < 128) {
					sampleHeight = 2*currentAcc;
				} else {
					sampleHeight = 2*(255-currentAcc);
				}
				break;
				
			case 2: // SQUARE
				// Max out sample height for the first half, then don't add anything for the second half
				if (currentAcc < 128) {
					sampleHeight = 255;
				}
				break; 
			
			case 3: // SAW
				// Increase the sample height until the phase accumulator overflows
				sampleHeight = currentAcc;
				break;
		}
        totalSampleHeight += sampleHeight;
	}
	// set the PWM duty cycle to the average sample height
	setDutyCycle(totalSampleHeight/4);
}