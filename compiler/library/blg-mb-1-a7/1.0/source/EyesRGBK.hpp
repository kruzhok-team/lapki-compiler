#pragma once

#include "RgbLed.hpp"

class EyesRGBK {

    uint8_t pin;

public:

    bool value;

    EyesRGBK() {}

    // ctor
    EyesRGBK(const uint8_t ledPin) {

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

    void setColor(const uint8_t red, const uint8_t green, const uint8_t blue, const uint8_t black) {

        if (!red && !green && !blue && !black)
            return;

        value = true;
        
        if (pin == 1) {

            detail::ReservedColor1 = detail::Color{ red, green, blue, black };
            mrx::hal::rgbLed::registerPin(pin, &detail::ReservedColor1);
        }
        else if (pin == 2) {

            detail::ReservedColor2 = detail::Color{ red, green, blue, black };
            mrx::hal::rgbLed::registerPin(pin, &detail::ReservedColor2);
        }
    }

    void off() {

        value = false;

        mrx::hal::rgbLed::unregisterPin(pin);
    }
};