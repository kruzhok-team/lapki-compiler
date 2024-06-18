#include "DigitalIn.h"

DigitalIn::DigitalIn(uint8_t pin){
    _pin = pin;
    _oldValue = 0;
    value = 0;
}

bool DigitalIn::isChanged(){
    value = digitalRead(_pin);
    
    return _oldValue != value;
}
