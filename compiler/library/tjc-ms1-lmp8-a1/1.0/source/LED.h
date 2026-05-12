#pragma once
#include "LEDController.hpp"
#include "const.h"
#include <cstdint>
#define USE_LED

class LED
{

public:
    unsigned int blinkLightInterval = 0; // сколько светим
    unsigned int blinkOffInterval = 0; // сколько выключены
    unsigned int currentBlinkInterval = 0; // сколько длится текущая фаза
    byte currentBlink = 0; // сколько всего миганий сделали
    byte overallBlinks = 0; // сколько всего миганий нужно сделать
    bool isBlinking = false; // мигаем ли сейчас
    bool isLighting = false; // горим сейчас или нет
    unsigned long startTime; // когда начали текущую фазу

    LED(uint8_t ledPin)
        {
            pwmIndex = ledPin - 1; // Index at PWM array
            value = 0;             // Брать из LEDController?
    }

    void on(const uint8_t brightness = MAX_LED_BRIGHTNESS, const bool offBlinking = true)
    {
        if (offBlinking) {
            isBlinking = false;
        }
        value = brightness;
        LEDController::set(pwmIndex, value);
    }

    void off(const bool offBlinking = true)
    {
        if (offBlinking) {
            isBlinking = false;
        }
        value = 0;
        LEDController::set(pwmIndex, 0);
    }

    void toggle(const bool offBlinking = true)
    {
        value > 0 ? off(offBlinking) : on(MAX_LED_BRIGHTNESS, offBlinking);
    }

    void blinking() {
        if (!isBlinking) return;
        // Если время интервала еще не прошло, просто обновляем время проверки
        if (millis() - startTime < currentBlinkInterval) {
            return;
        }
        // меняем фазу
        if (!isLighting) {
            // Считаем, что цикл закончился, когда переключаемся
            // с выкл на вкл
            isLighting = true;
            currentBlink += 1;
            currentBlinkInterval = blinkLightInterval;
        } else {
            isLighting = false;
            currentBlinkInterval = blinkOffInterval;
        }

        if (currentBlink >= overallBlinks) {
            isBlinking = false;
            return;
        }

        startTime = millis();
        toggle(false);
    }

    void blink(unsigned int lightInterval, unsigned int offInterval, byte times = 1) {
        for (byte i = 0; i < times; i++)
        {
            on();
            delay(lightInterval);
            off();
            delay(offInterval);
        }
    }

    void async_blink(unsigned int lightInterval, unsigned int offInterval, byte times = 1) {
        blinkOffInterval = offInterval;
        blinkLightInterval = lightInterval;
        overallBlinks = times;
        currentBlink = 0;
        isBlinking = true;
        isLighting = true;

        on(MAX_LED_BRIGHTNESS, false);
        currentBlinkInterval = lightInterval;
        startTime = millis();
    }

    uint8_t value;

private:
    LEDPin ledPinInfo;
    uint8_t pwmIndex;
};
