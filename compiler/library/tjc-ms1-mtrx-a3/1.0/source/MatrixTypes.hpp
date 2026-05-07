#pragma once

namespace detail {

    namespace constants {

        const uint8_t NUM_LEDS = 25;
        const uint8_t ROW_SZ = 5;
        const uint8_t COL_SZ = 5;
    }

    struct Led_t {

        GPIO_TypeDef* port;
        uint8_t num;

        bool state;
    };

    Led_t leds[detail::constants::NUM_LEDS];

    namespace service {

        void setupPinDiod(const Led_t& led) {

            led.port->MODER &= ~ ( 0b11 << ( GPIO_MODER_MODE0_Pos + led.num * 2U ));    // reset pin mode
            led.port->MODER |= ( 0b01 << ( GPIO_MODER_MODE0_Pos + led.num * 2U ));  // set general purpose mode (GP output mode)
            led.port->OTYPER |=( 0b01 << ( GPIO_OTYPER_OT0_Pos + led.num ));    // output mode pin (open drain)
            led.port->PUPDR &= ~ ( 0b11 << ( GPIO_PUPDR_PUPD0_Pos + led.num * 2U ));    // no pull-up, no pull-down
            led.port->BSRR |= ( 0b01 << ( GPIO_BSRR_BS0_Pos + led.num ));    // set bit on ODR
        }

        void onLed(Led_t& led) {
            led.port->BSRR |= (0b01 << (GPIO_BSRR_BR0_Pos + led.num));

            led.state = true;
        }

        void offLed(Led_t& led) {
            led.port->BSRR |= (0b01 << (GPIO_BSRR_BS0_Pos + led.num));

            led.state = false;
        }
    }

    namespace init {

        void initLeds() {

            // тактирование портов A, B, C, D (RM, p.152)
            RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
            RCC->IOPENR |= RCC_IOPENR_GPIOBEN;
            RCC->IOPENR |= RCC_IOPENR_GPIOCEN;
            RCC->IOPENR |= RCC_IOPENR_GPIODEN;

            // set-up pins
            int i = 0;

            // row1
            leds[i].port = GPIOC; leds[i].num = 6; detail::service::setupPinDiod(leds[i++]);   // led 1.1
            leds[i].port = GPIOC; leds[i].num = 7; detail::service::setupPinDiod(leds[i++]);   // led 1.2
            leds[i].port = GPIOA; leds[i].num = 11; detail::service::setupPinDiod(leds[i++]); // led 1.3
            leds[i].port = GPIOA; leds[i].num = 12; detail::service::setupPinDiod(leds[i++]); // led 1.4
            leds[i].port = GPIOB; leds[i].num = 9; detail::service::setupPinDiod(leds[i++]);   // led 1.5

            // row2
            leds[i].port = GPIOA; leds[i].num = 5; detail::service::setupPinDiod(leds[i++]);   // led 2.1
            leds[i].port = GPIOA; leds[i].num = 6; detail::service::setupPinDiod(leds[i++]);   // led 2.2
            leds[i].port = GPIOA; leds[i].num = 7; detail::service::setupPinDiod(leds[i++]);   // led 2.3
            leds[i].port = GPIOD; leds[i].num = 3; detail::service::setupPinDiod(leds[i++]);   // led 2.4
            leds[i].port = GPIOB; leds[i].num = 8; detail::service::setupPinDiod(leds[i++]);   // led 2.5

            // row3
            leds[i].port = GPIOB; leds[i].num = 10; detail::service::setupPinDiod(leds[i++]); // led 3.1
            leds[i].port = GPIOB; leds[i].num = 2; detail::service::setupPinDiod(leds[i++]);   // led 3.2
            leds[i].port = GPIOB; leds[i].num = 0; detail::service::setupPinDiod(leds[i++]);   // led 3.3
            leds[i].port = GPIOB; leds[i].num = 3; detail::service::setupPinDiod(leds[i++]);   // led 3.4
            leds[i].port = GPIOB; leds[i].num = 7; detail::service::setupPinDiod(leds[i++]);   // led 3.5

            // row4
            leds[i].port = GPIOB; leds[i].num = 12; detail::service::setupPinDiod(leds[i++]); // led 4.1
            leds[i].port = GPIOB; leds[i].num = 11; detail::service::setupPinDiod(leds[i++]); // led 4.2
            leds[i].port = GPIOB; leds[i].num = 13; detail::service::setupPinDiod(leds[i++]); // led 4.3
            leds[i].port = GPIOB; leds[i].num = 4; detail::service::setupPinDiod(leds[i++]);   // led 4.4
            leds[i].port = GPIOB; leds[i].num = 6; detail::service::setupPinDiod(leds[i++]);   // led 4.5


            // row5
            leds[i].port = GPIOA; leds[i].num = 8; detail::service::setupPinDiod(leds[i++]);   // led 5.1
            leds[i].port = GPIOB; leds[i].num = 15; detail::service::setupPinDiod(leds[i++]); // led 5.2
            leds[i].port = GPIOB; leds[i].num = 14; detail::service::setupPinDiod(leds[i++]); // led 5.3
            leds[i].port = GPIOB; leds[i].num = 5; detail::service::setupPinDiod(leds[i++]);   // led 5.4
            leds[i].port = GPIOC; leds[i].num = 13; detail::service::setupPinDiod(leds[i++]); // led 5.5
        }
    }
}

/*
    Pattern service
*/

// Этот тип строго АГРЕГАТ (см. функцию Matrix::setPattern(...))
struct Pattern {

    uint8_t a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25;
};

struct Pattern5 {

    uint8_t a1, a2, a3, a4, a5;
};

/* 
    init function
*/

auto&& init = []() -> int {

    detail::init::initLeds();
    return 0;
}();


// Тип операнда
enum Operand {

    mask_and,
    mask_or,
    mask_xor,
};