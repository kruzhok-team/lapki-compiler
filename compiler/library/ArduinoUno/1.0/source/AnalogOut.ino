#include "AnalogOut.h"

AnalogOut::AnalogOut(uint8_t pin) {
    _pin = pin;
}

void AnalogOut::write(int _value) {
    value = _value;
    analogWrite(_pin, _value);
}