#pragma once

#include "RgbLed.hpp"

class Eyes {

#define OFF_LEFT 1
#define OFF_RIGHT 2 
#define OFF_BOTH 3

public:
    uint8_t _pin = 0;
    // ctor
    Eyes() {}
    Eyes(uint8_t pin) {

        if (!detail::rgbLed::isInit) {

            // Инициализация модуля компонента
            mrx::hal::pwm::enablePWMTIM2();

            detail::rgbLed::isInit = true;
        }
        _pin = pin;
        mrx::hal::rgbLed::initPin(_pin);
    }

    void setColorPalette(detail::Color* color) {
        if (color == &ColorBlack) {
            mrx::hal::rgbLed::unregisterPin(_pin);
        }

        mrx::hal::rgbLed::registerPin(_pin, color);
    }


    void off() {
        mrx::hal::rgbLed::unregisterPin(_pin);
    }
};