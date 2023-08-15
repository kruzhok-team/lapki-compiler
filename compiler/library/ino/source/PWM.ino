#include "PWM.h"

PWM::PWM(uint8_t pin){
    _pin = pin;
}

void PWM::write(int value){ 
    analogWrite(_pin, value);
}