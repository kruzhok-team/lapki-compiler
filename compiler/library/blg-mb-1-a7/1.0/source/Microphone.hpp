#pragma once

namespace detail {

    namespace microphone {

        bool isInit = false;
    }
}

class Microphone {

public:
    uint16_t rValue = 0;
    uint16_t lValue = 0;
    bool mode = 0;
    // ctor
    Microphone() {
        if (!detail::microphone::isInit) {

            mrx::hal::microphone::api::init();

            detail::microphone::isInit = true;
            GPIOD->BSRR |= (0b01 << (GPIO_BSRR_BR0_Pos + 1));
        }
    }

    void on() {
        mode = 1;
    }

    void off() {
        mode = 0;
        lValue = 0;
        rValue = 0;
    }

    void scan () {
        if (mode) {
            senseLeft();
            senseRight();
            // GPIOD->BSRR |= (0b01 << (GPIO_BSRR_BR0_Pos + 1));
        }
    }

    void senseLeft() {
        lValue = mrx::hal::microphone::api::senseLeft();
    }

    void senseRight() {
        rValue = mrx::hal::microphone::api::senseRight();
    }
};