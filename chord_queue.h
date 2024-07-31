#include <avr/io.h>

#define MAX_CHORDS 4
#define NOTES_PER_CHORD 1
#define MAX_AMPLITUDE 255


typedef struct chord {
    uint16_t freq0;
    //int freq1;
    //int freq2;
    uint8_t amplitude;
    uint8_t stillHeld;
    uint8_t buttonIndex;
} chord_t;

//typedef int chord_t;


int getNumChords();
chord_t getChord(uint8_t index);
void playChord(int freq, uint8_t buttonIndex);

void releaseChord(int buttonIndex);
void manageChords();