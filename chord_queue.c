#include "chord_queue.h"

/*
    his is a custom data structure which holds a certain number of notes.
    This is important for letting notes exist on top of each other while fading in/out.

*/

chord_t queue[MAX_CHORDS] = {0, 0, 0, 0};

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

void playChord(int frequency, uint8_t buttonIndex) {
    chord_t newChord;
    newChord.freq0 = frequency;
    newChord.amplitude = 100;
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


void releaseChord(int buttonIndex) {
    for (int i = 0; i < MAX_CHORDS; i++) {
        if (queue[i].buttonIndex == buttonIndex && queue[i].stillHeld) {
            //queue[i].stillHeld = 0;
        }
    }
}


void manageChords() {
    for (int i = 0; i < MAX_CHORDS; i++) {
        if (queue[i].stillHeld) {
            queue[i].amplitude = 1;/*
            if (queue[i].amplitude < MAX_AMPLITUDE) {
                queue[i].amplitude += 1;
            } else {
                queue[i].amplitude = MAX_AMPLITUDE;
            }*/
        /*} else {
            if (queue[i].amplitude > 0) {
                queue[i].amplitude -= 1;
            } else {
                queue[i].amplitude = 0;
            }*/
        }
    }
}