#include "bitwise.h"

uint16_t setBit(uint16_t bitString, uint8_t bitToSet) {
    return bitString | (1 << bitToSet);
}

uint16_t clearBit(uint16_t bitString, uint8_t bitToClear) {
    return bitString & ~(1 << bitToClear);
}

uint8_t checkBit(uint16_t bitString, uint8_t bitToCheck) {
	return (bitString & (1 << bitToCheck)) > 0;
}