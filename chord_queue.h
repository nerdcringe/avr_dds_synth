
#define MAX_CHORDS 4
#define NOTES_PER_CHORD 1

/*
typedef struct chord {
    int freq0;
    //int freq1;
    //int freq2;
    int volume;
    int isRising;
} chord_t;*/

typedef int chord_t;


int getNumChords();
chord_t getChord(int index);
void insertChord(chord_t element);