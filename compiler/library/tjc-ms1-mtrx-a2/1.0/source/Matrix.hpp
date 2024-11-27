#pragma once

/*
    leds service
*/

namespace detail {

    namespace constants {

        const uint8_t NUM_LEDS = 25;
        const uint8_t ROW_SZ = 5;
        const uint8_t COL_SZ = 5;
    }

    struct Led_t {

        GPIO_TypeDef* port;
        uint8_t num;
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

        void onLed(const Led_t& led) {
            led.port->BSRR |= (0b01 << (GPIO_BSRR_BR0_Pos + led.num));
        }

        void offLed(const Led_t& led) {
            led.port->BSRR |= (0b01 << (GPIO_BSRR_BS0_Pos + led.num));
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

/* 
    init function
*/

auto&& init = []() -> int {

    detail::init::initLeds();
    return 0;
}();


// TODO descr
class Matrix {

public:

    // ctor
    Matrix() {
        clear();
    }

    // row in range [0 : details::constant::ROW_SZ]
    // col in range [0 : details::constant::COL_SZ]
    void setPixel(const uint8_t row, const uint8_t col, const uint8_t value) {

        setPixel(row * detail::constants::ROW_SZ + col, value);
    }

    // idx == linear index of led
    void setPixel(const uint8_t idx, const uint8_t value) {

        (void)value;
        detail::service::onLed(detail::leds[idx]);
    }

    void offPixel(const uint8_t row, const uint8_t col) {

        offPixel(row * detail::constants::ROW_SZ + col);
    }

    void offPixel(const uint8_t idx) {

        detail::service::offLed(detail::leds[idx]);
    }

    void setPattern(const Pattern& pattern) {

        const uint8_t* const ptrPattern = reinterpret_cast<const uint8_t* const>(&pattern);

        for (int i = 0; i < detail::constants::NUM_LEDS; ++i) {

            if (ptrPattern[i] > 0)
                setPixel(i, ptrPattern[i]);
            else {
                offPixel(i);
            }
        }
    }

    void fill(const uint8_t value) {

        for (int i = 0; i < detail::constants::NUM_LEDS; ++i) {

            setPixel(i, value);
        }
    }

    void clear() {
        
        for (int i = 0; i < detail::constants::NUM_LEDS; ++i) {

            offPixel(i);
        }
    }
};
