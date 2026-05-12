#pragma once

#include "PWM.hpp"
#define MAX_BRIGHTNESS 100

class LED {

private:
    uint8_t map(uint8_t x, uint8_t in_min, uint8_t in_max, uint8_t out_min, uint8_t out_max) {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

public:
    LED(uint8_t ledPin) {

        pin = ledPin + 4;   // map from user pin to stm32 pin {1,2,3} -> {5, 6, 7}

        RCC -> IOPENR |= RCC_IOPENR_GPIOAEN; // тактирование

        GPIOA->MODER &= ~(0b11 << (GPIO_MODER_MODE0_Pos + pin * 2U)); // reset pin mode
        GPIOA->MODER |= (0b01 << (GPIO_MODER_MODE0_Pos + pin * 2U));  // set general purpose mode (GP output mode)
        GPIOA->OTYPER &= ~(GPIO_OTYPER_OT0 << pin);       // output mode pin (PP)
        GPIOA->PUPDR &= ~(0b11 << (GPIO_PUPDR_PUPD0_Pos + pin * 2U)); // no pull-up, no pull-down
        GPIOA->BSRR |= (0b01 << (GPIO_BSRR_BS0_Pos + pin));           // set bit on ODR

        value = 0;
        off();
    }

    void on(const uint8_t brightness = MAX_BRIGHTNESS, const bool offBlinking = true) {
        if (offBlinking) {
            isBlinking = false;
        }
        // change state
        value = 1;

        // Если на всю яркость - все просто
        if (brightness == MAX_BRIGHTNESS) {
            GPIOA->BSRR |= ( GPIO_BSRR_BS0 << pin );
            return;
        }

        // Если яркость == 0, то выключаем светодиод (меняем состояние класса)
        if (brightness == 0) {
            off();
            return;
        }

        // Иначе подключаем ШИМ

        // mapping [0.255] -> [0..100]
        const uint8_t val = brightness; //const uint8_t val = map(brightness, 0, 255, 0, 100);

        PWM().write(val, pin -4);
    }

    void off(const bool offBlinking = true) {
        if (offBlinking) {
            isBlinking = false;
        }
        GPIOA->BSRR |= ( GPIO_BSRR_BR0 << pin );
        value = 0;

        // Выключаем ШИМ, если включение было через него
        PWM().write(0, pin - 4);
    }

    void toggle(const bool offBlinking = true) {
        value > 0 ? off(offBlinking) : on(MAX_BRIGHTNESS, offBlinking);
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

        on(MAX_BRIGHTNESS, false);
        currentBlinkInterval = lightInterval;
        startTime = millis();
    }

    bool value;

private:
    unsigned int blinkLightInterval = 0; // сколько светим
    unsigned int blinkOffInterval = 0; // сколько выключены
    unsigned int currentBlinkInterval = 0; // сколько длится текущая фаза
    byte currentBlink = 0; // сколько всего миганий сделали
    byte overallBlinks = 0; // сколько всего миганий нужно сделать
    bool isBlinking = false; // мигаем ли сейчас
    bool isLighting = false; // горим сейчас или нет
    unsigned long startTime; // когда начали текущую фазу
    uint8_t pin;
};
