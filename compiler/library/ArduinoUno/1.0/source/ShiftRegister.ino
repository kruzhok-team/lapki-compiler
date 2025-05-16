#include "ShiftRegister.h"

ShiftRegister::ShiftRegister(uint8_t dataPin, uint8_t clockPin, uint8_t latchPin, uint8_t bitOrder) {

    _dataPin = dataPin;
    _clockPin = clockPin;
    _latchPin = latchPin;
    _bitOrder = bitOrder;

    pinMode(_dataPin, OUTPUT);
    pinMode(_clockPin, OUTPUT);
    pinMode(_latchPin, OUTPUT);
}

void ShiftRegister::shift(const uint8_t value) {

    digitalWrite(_latchPin, LOW);

    shiftOut(_dataPin, _clockPin, _bitOrder, value);

    digitalWrite(_latchPin, HIGH);
}