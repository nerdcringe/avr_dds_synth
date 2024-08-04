#include "chord_queue.h"

/*
    his is a custom data structure which holds a certain number of notes.
    This is important for letting notes exist on top of each other while fading in/out.

*/

chord_t queue[MAX_CHORDS] = {0};

uint8_t attackRate = 12;
uint8_t releaseRate = 3;


uint8_t isEmpty(chord_t chord) {
    return chord.amplitude <= 0 && !chord.stillHeld;
}

chord_t getChord(uint8_t index) {
    return queue[index];
}



int getNumChords() {
    int num = 0;
    for (int i = 0; i < MAX_CHORDS; i++) {
        if (!isEmpty(queue[i])) {
            num++;
        }
    }
    return num;
}


void playChord(uint16_t freq0, uint16_t freq1, uint16_t freq2, uint8_t buttonIndex) {
    chord_t newChord;
    newChord.freq0 = freq0;
    newChord.freq1 = freq1;
    newChord.freq2 = freq2;
    newChord.amplitude = 1;
    newChord.stillHeld = 1;
    newChord.buttonIndex = buttonIndex;

    // Place number in first non-zero slot
    for (int i = 0; i < MAX_CHORDS; i++) {
        if (isEmpty(queue[i])) {
            queue[i] = newChord;
            return;
        }
    }
    // If no nonzero slots are available, shift everything one index left and put the new number at the end
    // If the note is old enough, it probably is almost faded out and might as well be replaced by the new one
    for (int i = 0; i < MAX_CHORDS - 1; i++) {
        queue[i] = queue[i+1];
    }
    queue[MAX_CHORDS - 1] = newChord;
}


void releaseChord(uint8_t buttonIndex) {
    for (int i = 0; i < MAX_CHORDS; i++) {
        if (queue[i].buttonIndex == buttonIndex && queue[i].stillHeld) {
            queue[i].stillHeld = 0;
        }
    }
}


void manageChords() {
    for (int i = 0; i < MAX_CHORDS; i++) {
        if (queue[i].stillHeld) {
            if (queue[i].amplitude < (MAX_AMPLITUDE - 1 - attackRate)) {
                queue[i].amplitude += attackRate;
            } else {
                queue[i].amplitude = MAX_AMPLITUDE - 1;
            }
        } else {
            if (queue[i].amplitude >= 1){//releaseRate + 1) {
                //queue[i].amplitude -= releaseRate;
                queue[i].amplitude = (queue[i].amplitude * (256-releaseRate))/256;
            } else {
                queue[i].amplitude = 1;
            }
        }
    }
}
