#pragma once

#include "RgbLed.hpp"

class Eyes {

#define OFF_LEFT 1
#define OFF_RIGHT 2 
#define OFF_BOTH 3

public:

    // ctor
    Eyes() {

        if (!detail::rgbLed::isInit) {

            // Инициализация модуля компонента
            mrx::hal::pwm::enablePWMTIM2();

            detail::rgbLed::isInit = true;
        }

        mrx::hal::rgbLed::initPin(1);
        mrx::hal::rgbLed::initPin(2);

        setColorPaletteRight(&ColorBlack);
        setColorPaletteLeft(&ColorBlack);
    }

    void setColorPallete(detail::Color* colorLeft, detail::Color* colorRight) {
        setColorPaletteLeft(colorLeft);
        setColorPaletteRight(colorRight);
    }

    void setColorPaletteRight(detail::Color* color) {

        if (color == &ColorBlack) {

            mrx::hal::rgbLed::unregisterPin(1);
        }

        mrx::hal::rgbLed::registerPin(1, color);
    }

    void setColorPaletteLeft(detail::Color* color) {
        if (color == &ColorBlack) {

            mrx::hal::rgbLed::unregisterPin(2);
        }

        mrx::hal::rgbLed::registerPin(2, color);
    }
    
    void off(uint8_t mode) {
        switch (mode)
        {
        case OFF_LEFT:
            mrx::hal::rgbLed::unregisterPin(2);
            break;
        case OFF_RIGHT:
            mrx::hal::rgbLed::unregisterPin(1);
            break;
        case OFF_BOTH:
            mrx::hal::rgbLed::unregisterPin(1);
            mrx::hal::rgbLed::unregisterPin(2);
            break;
        default:
            break;
        }
    }
};