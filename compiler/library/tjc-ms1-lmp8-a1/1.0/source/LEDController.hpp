#pragma once
#include <stdint.h>
#include "stm32g030xx.h"
#include "Pins.hpp"
#include "const.h"

struct LEDPin
{
    GPIO_TypeDef *port;
    uint8_t pin;
    uint8_t value;
    uint8_t counter;
};

LEDPin leds[8] = {
    {GPIOA, 9, 0, 0},  // 1
    {GPIOC, 6, 0, 0},  // 2
    {GPIOA, 11, 0, 0}, // 3
    {GPIOA, 10, 0, 0}, // 4
    {GPIOA, 5, 0, 0},  // 5
    {GPIOA, 6, 0, 0},  // 6
    {GPIOA, 7, 0, 0},  // 7
    {GPIOB, 0, 0, 0}   // 8
};

// Контроллер светодиодов, инициализируется вместе с ШИМ в setup();
// По таймеру вызывается `update`
class LEDController
{
private:
    static const uint8_t LED_COUNT = 8;

public:
    static void init()
    {
        RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
        RCC->IOPENR |= RCC_IOPENR_GPIOBEN;
        RCC->IOPENR |= RCC_IOPENR_GPIOCEN;

        // Инициализация пинов
        for (uint8_t i = 0; i < LED_COUNT; i++)
        {
            initPin_PP(leds[i].port, leds[i].pin);
            setPin_PP(leds[i].port, leds[i].pin, OFF);
        }
    }
    
    // Выставить яркость, эта функция и дергается в LED.h
    static void set(uint8_t pin, uint8_t value)
    {
        leds[pin].value = value;
    }
    
    // Выставить значение на пине
    static void setLed(uint8_t idx, uint8_t value)
    {
        setPin_PP(leds[idx].port, leds[idx].pin, value);
    }

    // ШИМ
    static void processLED(uint8_t idx)
    {
        if (leds[idx].counter >= leds[idx].value)
        {
            setLed(idx, OFF);
        }
        else
        {
            setLed(idx, ON);
        }

        leds[idx].counter++;
        leds[idx].counter %= MAX_LED_BRIGHTNESS;
    }

    // "Просчитать" ШИМ для каждого светодиода
    static void update()
    {
        for (uint8_t i = 0; i < LED_COUNT; i++)
        {
            processLED(i);
        }
    }
};