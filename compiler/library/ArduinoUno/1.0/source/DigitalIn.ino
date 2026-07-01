#include "DigitalIn.h"

#include <stdint.h>

DigitalIn::DigitalIn(uint8_t pin){
    _pin = pin;
    _oldValue = 0;
    value = 0;
}

bool DigitalIn::changed(){
    return _oldValue != value;
}

bool DigitalIn::lowToHigh() {
    return _oldValue == LOW && value == HIGH;
}

bool DigitalIn::highToLow() {
    return _oldValue == HIGH && value == LOW;
}

void DigitalIn::update(){
    _oldValue = value;
    value =  digitalRead(_pin);
}