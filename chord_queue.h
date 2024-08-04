#include <avr/io.h>

#define MAX_CHORDS 2
#define MAX_AMPLITUDE 256
#define FREQS_PER_CHORD 3


typedef struct chord {
    uint16_t freq0;
    uint16_t freq1;
    uint16_t freq2;
    uint16_t amplitude;
    uint8_t stillHeld;
    uint8_t buttonIndex;
} chord_t;

//typedef int chord_t;


chord_t getChord(uint8_t index);
int getNumChords();
void playChord(uint16_t freq0, uint16_t freq1, uint16_t freq2, uint8_t buttonIndex);

void releaseChord(uint8_t buttonIndex);
void manageChords();