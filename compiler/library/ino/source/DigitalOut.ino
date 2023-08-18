#include "DigitalOut.h"

DigitalOut::DigitalOut(uint8_t _pin) {
    pin = _pin;
}

void DigitalOut::high() {
    digitalWrite(pin, HIGH);
}

void DigitalOut::low() {
    digitalWrite(pin, LOW);
}

void DigitalOut::init() {
    pinMode(pin, OUTPUT);    
}

void DigitalOut::setPWM(int value) {
    analogWrite(pin, value);
}
