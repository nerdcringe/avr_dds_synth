#include "chord_queue.h"

/*
    his is a custom data structure which holds a certain number of notes.
    This is important for letting notes exist on top of each other while fading in/out.

*/

int queue[MAX_CHORDS] = {0, 0, 0, 0};


int getNumChords() {
    int num = 0;
    for (int i = 0; i < MAX_CHORDS; i++) {
        if (queue[i] != 0) {
            num++;
        }
    }
    return num;
}

int getChord(int index) {
    return queue[index];
}


void insertChord(int element) {
    // Place number in first non-zero slot
    for (int i = 0; i < MAX_CHORDS; i++) {
        if (queue[i] == 0) {
            queue[i] = element;
            return;
        }
    }
    // If no nonzero slots are available, shift everything one index left and put the new number at the end
    // If the note is old enough, it probably is almost faded out and might as well be replaced by the new one
    for (int i = 0; i < MAX_CHORDS - 1; i++) {
        queue[i] = queue[i+1];
    }
    queue[MAX_CHORDS - 1] = element;
}