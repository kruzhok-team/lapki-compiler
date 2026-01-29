#pragma once

#include "PWM.hpp"

struct LEDPin
{
    GPIO_TypeDef *port;
    uint8_t pin;
    uint8_t RCC_OPEN;
};

class LED
{

private:
LEDPin ledPins[8] = {
    {GPIOA, 9, RCC_IOPENR_GPIOAEN},  // 1
    {GPIOC, 6, RCC_IOPENR_GPIOCEN},  // 2
    {GPIOA, 11, RCC_IOPENR_GPIOAEN}, // 3
    {GPIOA, 10, RCC_IOPENR_GPIOAEN}, // 4
    {GPIOA, 5, RCC_IOPENR_GPIOAEN},  // 5
    {GPIOA, 6, RCC_IOPENR_GPIOAEN},  // 6
    {GPIOA, 7, RCC_IOPENR_GPIOAEN},  // 7
    {GPIOB, 0, RCC_IOPENR_GPIOBEN}   // 8
};

unsigned int blinkLightInterval = 0; // сколько светим
unsigned int blinkOffInterval = 0; // сколько выключены
unsigned int currentBlinkInterval = 0; // сколько длится текущая фаза
byte currentBlink = 0; // сколько всего миганий сделали
byte overallBlinks = 0; // сколько всего миганий нужно сделать
bool isBlinking = false; // мигаем ли сейчас
bool isLighting = false; // горим сейчас или нет
unsigned long startTime; // когда начали текущую фазу


public : LED(uint8_t ledPin)
    {
        pwmIndex = ledPin - 1; // Index at PWM array
        ledPinInfo = ledPins[pwmIndex];
        RCC->IOPENR |= ledPinInfo.RCC_OPEN; // тактирование
        initPin_PP(ledPinInfo.port, ledPinInfo.pin);

        value = 0;
        off();
    }

    bool getState()
    {

        return value;
    }

    void on(const uint8_t brightness = 100)
    {

        // change state
        value = 1;

        // Если на всю яркость - все просто
        /*
        if (brightness == 100)
        {
            ledPinInfo.port->BSRR |= (GPIO_BSRR_BS0 << ledPinInfo.pin);
            return;
        }
        */

        // Если яркость == 0, то выключаем светодиод (меняем состояние класса)
        if (brightness == 0)
        {
            off();
            return;
        }

        // Иначе подключаем ШИМ

        // mapping [0.255] -> [0..100]
        const uint8_t val = brightness; // const uint8_t val = map(brightness, 0, 255, 0, 100);

        PWM().write(val, pwmIndex);
    }

    void off()
    {

        ledPinInfo.port->BSRR |= (GPIO_BSRR_BR0 << ledPinInfo.pin);
        value = 0;

        // Выключаем ШИМ, если включение было через него
        PWM().write(0, pwmIndex);
    }

    void toggle()
    {

        value ? off() : on();
    }

    void blinking() {
        if (!isBlinking) return;
        // Если время интервала еще не прошло, просто обновляем время проверки
        if (millis() - startTime < currentBlinkInterval) {
            return;
        }
        unsigned int interval = 0;
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

        if (currentBlink > overallBlinks) {
            isBlinking = false;
            return;
        }

        startTime = millis();
        toggle();
    }

    void blink(unsigned int lightInterval, unsigned int offInterval, byte times = 1) {
        blinkOffInterval = offInterval;
        blinkLightInterval = lightInterval;
        overallBlinks = times;
        currentBlink = 0;
        isBlinking = true;

        on();
        currentBlinkInterval = lightInterval;
        startTime = millis();
    }


    void setValue(byte val)
    {

        value = (val <= 127) ? 0 : 1;
        toggle();
        toggle();
    }

    void fadeIn(unsigned int time)
    {

        return;
    }

    void fadeOut(unsigned int time)
    {

        return;
    }

    bool value;

private:
    LEDPin ledPinInfo;
    uint8_t pwmIndex;
};
