#include "Button.h"
#include <iostream>
Button::Button(int pin){
    _pin = pin;
}

bool Button::isJustPressed(){
    std::cout << "Button pressed!" << std::endl;
    return true;
}