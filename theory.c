#include "theory.h"
#include <math.h>


#define I_CHORD 0
#define ii_CHORD 1
#define iii_CHORD 2
#define IV_CHORD 3
#define V_CHORD 4
#define vi_CHORD 5
#define vii_CHORD 6



 // Tonic note is the home note for a given key. For example, the tonic in the key of A is just A.
 // Every note in a key is defined in relation to the tonic, so changing the tonic changes the whole key.
int tonicFreqs[12] = {
	3520, 3729, 3951, 4186,
	4435, 4699, 4978, 5274,
	5588, 5920, 6272, 6645
};



void getMainFreqs(uint8_t keyIndex, uint8_t chordNum, uint8_t swapMajMin, uint8_t inversionNum,
    uint16_t* lowFreq, uint16_t* midFreq, uint16_t* highFreq) {
    
	uint16_t tonicFreq = tonicFreqs[keyIndex % 12];
    uint16_t rootFreq = 0;
    uint16_t thirdFreq = 0;
    uint16_t fifthFreq = 0;

    // booleans that determine the structure of the chord
    uint8_t minorThird = 0;
    uint8_t diminishedFifth = 0;
    
    switch (chordNum) {
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

    if (swapMajMin) {
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
    *lowFreq = fmin(rootFreq, fmin(thirdFreq, fifthFreq));
    *highFreq = fmax(rootFreq, fmax(thirdFreq, fifthFreq));
    if (rootFreq > *lowFreq && rootFreq < *highFreq) {
        *midFreq = rootFreq;
    } else if (thirdFreq > *lowFreq && thirdFreq < *highFreq) {
        *midFreq = thirdFreq;
    } else {
        *midFreq = fifthFreq;
    }

    // As the analog input value increases, each pitch goes down an octave one-by-one in a cycle.
    // The highest pitch goes down each time, which is why they needed to be sorted by pitch previously.
    for (int i = 0; i < inversionNum; i++) {
        if (i % 3 == 2) {
            *lowFreq /= 2;
        }
        if (i % 3 == 1) {
            *midFreq /= 2;
        }
        if (i % 3 == 0) {
            *highFreq /= 2;
        }
    }
    }