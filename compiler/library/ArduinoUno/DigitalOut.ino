#include "DigitalOut.h"

DigitalOut::DigitalOut(uint8_t _pin) {
    pin = _pin;
    value = LOW;
}

//
void DigitalOut::on() {
    digitalWrite(pin, HIGH);
    value = HIGH;
}

//
void DigitalOut::off() {
    digitalWrite(pin, LOW);
    value = LOW;
}

void DigitalOut::init() {
    pinMode(pin, OUTPUT);    
}

void DigitalOut::toggle(){
    if(value == LOW) {
        digitalWrite(pin, HIGH);
    }
    else {
        digitalWrite(pin, LOW);
    }
}
