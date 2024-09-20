#ifndef LED_H
#define LED_H

class LED {

public:
    LED(uint8_t ledPin) {

        pin = ledPin + 4;   // map from user pin to stm32 pin {1,2,3} -> {5, 6, 7}

        RCC -> IOPENR |= RCC_IOPENR_GPIOAEN; // тактирование

        GPIOA->MODER &= ~(0b11 << (GPIO_MODER_MODE0_Pos + pin * 2U)); // reset pin mode
        GPIOA->MODER |= (0b01 << (GPIO_MODER_MODE0_Pos + pin * 2U));  // set general purpose mode (GP output mode)
        GPIOA->OTYPER |= (0b01 << (GPIO_OTYPER_OT0_Pos + pin));       // output mode pin (open drain)
        GPIOA->PUPDR &= ~(0b11 << (GPIO_PUPDR_PUPD0_Pos + pin * 2U)); // no pull-up, no pull-down
        GPIOA->BSRR |= (0b01 << (GPIO_BSRR_BS0_Pos + pin));           // set bit on ODR

        value = 0;
        off();
    }

    bool getState() {

        return value;
    }

    void on() {

        GPIOA->BSRR |= (0b01 << (GPIO_BSRR_BR0_Pos + pin));
        value = 1;
    }

    void off() {

        GPIOA->BSRR |= (0b01 << (GPIO_BSRR_BS0_Pos + pin));
        value = 0;
    }

    void toggle() {

        value ? off() : on();
    }

    void blink(unsigned int time, byte times = 1) {

        for (byte i = 0; i < times; i++)
        {
            toggle();
            delay(time / 2);
            toggle();
            delay(time / 2);
        }
    }

    void setValue(byte val) {

        value = (val <= 127) ? 0 : 1;
        toggle();
        toggle();
    }

    void fadeIn(unsigned int time) {

        return;
    }

    void fadeOut(unsigned int time) {

        return;
    }

    bool value;

private:
    uint8_t pin;
};

#endif
// LED_H