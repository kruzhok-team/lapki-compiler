#include "Button.h"
#include <iostream>
Button::Button(int pin){
    _pin = pin;
}

bool Button::isJustPressed(){
    return true;
}