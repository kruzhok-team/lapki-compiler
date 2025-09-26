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

    void blink(unsigned int lightInterval, unsigned int offInterval, byte times = 1) {

        for (byte i = 0; i < times; i++)
        {
            toggle();
            delay(lightInterval);
            toggle();
            delay(offInterval);
        }
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