#pragma once

#include "RgbLed.hpp"

class EyesPalette {

    uint8_t pin;

public:

    bool value;

    EyesPalette() {}

    // ctor
    EyesPalette(const uint8_t ledPin) {

        if (!detail::rgbLed::isInit) {

            // Инициализация модуля компонента
            mrx::hal::pwm::enablePWMTIM2();

            detail::rgbLed::isInit = true;
        }
        
        pin = ledPin;

        if (pin < mrx::hal::rgbLed::minPin || pin > mrx::hal::rgbLed::maxPin)
            pin = mrx::hal::rgbLed::minPin;

        mrx::hal::rgbLed::initPin(pin);

        off();
    }

    void setColorPalette(detail::Color* color) {

        value = true;

        mrx::hal::rgbLed::registerPin(pin, color);
    }

    void off() {

        value = false;

        mrx::hal::rgbLed::unregisterPin(pin);
    }
};