#include "Led.h"
#include <iostream>
Led::Led(int pin){
    _pin = pin;
}

void Led::turnOff(){
    std::cout << "turnOff!" << std::endl;
}

void Led::turnOn(){
    std::cout << "turnOn!" << std::endl;
}