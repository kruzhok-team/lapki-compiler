#pragma once
#include "LEDController.hpp"
#include "const.h"
#define USE_LED
class LED
{
public:
    LED(uint8_t ledPin)
    {
        pwmIndex = ledPin - 1; // Index at PWM array
        value = 0;             // Брать из LEDController?
    }

    bool getState()
    {
        return value;
    }

    void on(const uint8_t brightness = MAX_LED_BRIGHTNESS)
    {
        value = brightness;
        LEDController::set(pwmIndex, value);
    }

    void off()
    {
        LEDController::set(pwmIndex, 0);
    }

    void toggle()
    {

        value ? off() : on();
    }

    void blink(unsigned int time, byte times = 1)
    {

        for (byte i = 0; i < times; i++)
        {
            toggle();
            delay(time / 2);
            toggle();
            delay(time / 2);
        }
    }

    uint8_t value;

private:
    LEDPin ledPinInfo;
    uint8_t pwmIndex;
};