#pragma once

/*
    Файл коннектор для главной платы и кнопочной платы (main-a4, btn-a2)
    Он содержит пины, которые доступны для ШИМ,
    а также функции, необходимые для их активации или деактивации
*/



namespace Connector {

    namespace actFuncs {

        // on or off leds

        void led1(const bool isActive) {
            
            if (isActive)
                GPIOA->BSRR |= ( GPIO_BSRR_BS0 << 9 );
            else
                GPIOA->BSRR |= ( GPIO_BSRR_BR0 << 9 );
        }

        void led2(const bool isActive) {
            
            if (isActive)
                GPIOC->BSRR |= ( GPIO_BSRR_BS0 << 6 );
            else
                GPIOC->BSRR |= ( GPIO_BSRR_BR0 << 6 );
        }

        void led3(const bool isActive) {
            
            if (isActive)
                GPIOA->BSRR |= ( GPIO_BSRR_BS0 << 11 );
            else
                GPIOA->BSRR |= ( GPIO_BSRR_BR0 << 11 );
        }

        void led4(const bool isActive) {
            
            if (isActive)
                GPIOA->BSRR |= ( GPIO_BSRR_BS0 << 10 );
            else
                GPIOA->BSRR |= ( GPIO_BSRR_BR0 << 10 );
        }
        void led5(const bool isActive) {
            
            if (isActive)
                GPIOA->BSRR |= ( GPIO_BSRR_BS0 << 5 );
            else
                GPIOA->BSRR |= ( GPIO_BSRR_BR0 << 5 );
        }
        void led6(const bool isActive) {
            
            if (isActive)
                GPIOA->BSRR |= ( GPIO_BSRR_BS0 << 6 );
            else
                GPIOA->BSRR |= ( GPIO_BSRR_BR0 << 6 );
        }
        void led7(const bool isActive) {
            
            if (isActive)
                GPIOA->BSRR |= ( GPIO_BSRR_BS0 << 7 );
            else
                GPIOA->BSRR |= ( GPIO_BSRR_BR0 << 7 );
        }
        void led8(const bool isActive) {
            
            if (isActive)
                GPIOB->BSRR |= ( GPIO_BSRR_BS0 << 0 );
            else
                GPIOB->BSRR |= ( GPIO_BSRR_BR0 << 0 );
        }
    }

    using actFuncType = void(*)(bool);

    struct Entity {
        uint8_t pin;
        actFuncType actFunc;
    };

    const int SIZEBuf = 8;
    const Entity pwmPins[] = {
        {0, actFuncs::led1},
        {1, actFuncs::led2},
        {2, actFuncs::led3},
        {3, actFuncs::led4},
        {4, actFuncs::led5},
        {5, actFuncs::led6},
        {6, actFuncs::led7},
        {7, actFuncs::led8}
    };

    namespace Api {

        bool isPwmPin(const uint8_t pin) {

            for (int i = 0; i < Connector::SIZEBuf; ++i) {

                if (pwmPins[i].pin == pin)
                    return true;
            }

            return false;
        }

        actFuncType getActFunc(const uint8_t pin) {

            for (int i = 0; i < Connector::SIZEBuf; ++i) {

                if (pwmPins[i].pin == pin)
                    return pwmPins[i].actFunc;
            }
            
            return nullptr;
        }
    }
}